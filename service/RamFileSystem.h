//-----------------------------------------------------------------------------
// Copyright (c) 2014 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef __RAMFILESYSTEM_H_
#define __RAMFILESYSTEM_H_
#pragma once

#include <atomic>
#include <concurrent_queue.h>
#include <mutex>
#include "FileSystem.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)		// inline specifier cannot be used (friend)

//-----------------------------------------------------------------------------
// Class RamFileSystem
//
// Implements an in-memory file system
//
// Limits: INT32_MAX inodes
// todo: document more

class RamFileSystem : public FileSystem
{
public:

	// Destructor
	//
	virtual ~RamFileSystem()=default;

	class Directory : public FileSystem::Directory
	{
	public:

		// Destructor
		//
		virtual ~Directory()=default;

	private:

		Directory(const Directory&)=delete;
		Directory& operator=(const Directory&)=delete;

		Directory()=default;
	};

	// Class Node
	//
	// Node implementation for the file system
	class Node : public FileSystem::Node
	{
	public:

		// Constructor / Destructor
		//
		// TODO: needs type, mode and uid/gid arguments
		Node(RamFileSystem& fs, int32_t index);
		virtual ~Node();

		// (FileSystem::Node)

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

		// m_fs
		//
		// Reference to the parent file system
		RamFileSystem& m_fs;

		// m_blocks
		//
		// TODO: this will hold the extent:block indexes

		// m_lock
		//
		// Block data read/write access lock
		std::recursive_mutex m_lock;
	};

	// this represents a view of the node by a process
	//class File : public FileSystem::File
	//{
	//public:

	//	// Destructor
	//	//
	//	virtual ~File()=default;

	//private:

	//	File(const File&)=delete;
	//	File& operator=(const File&)=delete;

	//	File()=default;
	//};

	// Mount (static)
	//
	// Mounts the file system on the specified device, returns the FileSystem instance
	static std::unique_ptr<FileSystem> Mount(int flags, const char_t* devicename, void* data);

private:

	RamFileSystem(const RamFileSystem&)=delete;
	RamFileSystem& operator=(const RamFileSystem&)=delete;

	// Instance Constructor
	RamFileSystem(size_t max);
	friend std::unique_ptr<RamFileSystem> std::make_unique<RamFileSystem, size_t&>(size_t&);

	// Class Extent
	//
	// Implements a chunk of virtual memory, divided up into blocks of data
	// based on the system page size.  Allocation/release of individual blocks 
	// is atomic, however access to the block data must be properly serialized 
	// externally to this class.
	//
	// The number of blocks is limited to 32768 (INT16_MAX) to allow for a single 
	// 32-bit variable to hold an EXTENT:BLOCK memory index in the outer class, this
	// caps a single extent to 128MiB with standard 4K memory pages
	class Extent
	{
	public:

		// Destructor
		~Extent();

		// AllocateBlock
		//
		// Allocates (commits) a single block from this chunk
		int16_t AllocateBlock(void);
		void* AllocateBlock(int16_t& index);

		// Create (static)
		//
		// Create a new Chunk with the specified number of blocks
		static std::unique_ptr<Extent> Create(int16_t blocks);

		// ReleaseBlock
		//
		// Releases (decommits) a single block in this chunk
		void ReleaseBlock(int16_t index);

		// Block
		//
		// Gets the base address of an allocated block
		__declspec(property(get=getBlock)) void* Block[];
		void* getBlock(int16_t index) const;

		// BlockSize (static)
		//
		// The number of bytes in a single block
		static const size_t BlockSize;

		// TotalBlocks
		//
		// The total number of blocks reserved for this chunk
		__declspec(property(get=getTotalBlocks)) int16_t TotalBlocks;
		int16_t getBlocks(void) const { return m_total; }

	private:

		Extent(const Extent&)=delete;
		Extent& operator=(const Extent&)=delete;

		// Instance Constructor
		Extent(void* base, int16_t blocks) : m_base(uintptr_t(base)), m_total(blocks) {}
		friend std::unique_ptr<Extent> std::make_unique<Extent, void*&, int16_t&>(void*&, int16_t&);

		// ValidateBlock
		//
		// Validates the block index and in DEBUG builds the memory state
		bool ValidateBlockState(void* address, DWORD state) const;

		// m_base
		//
		// Base address of the chunk memory reservation
		uintptr_t m_base;

		// m_nextblock
		//
		// Atomic counter indicating the next sequential unallocated block
		std::atomic<int16_t> m_nextblock = 0;

		// m_spentblocks
		//
		// Queue used to store released block indexes for reuse
		Concurrency::concurrent_queue<int16_t> m_spentblocks;

		// m_total
		//
		// Total number of blocks reserved for this chunk
		int16_t	m_total;
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AllocateBlock
	//
	// Allocates a block of memory to use for node data
	uint32_t AllocateBlock(void);
	void* AllocateBlock(uint32_t& block);

	// AllocateNodeIndex
	//
	// Allocates a node index from the pool
	int32_t AllocateNodeIndex(void);

	// ReleaseBlock
	//
	// Releases a block of data back into the block pool
	void ReleaseBlock(uint32_t block);

	// ReleaseNodeIndex
	//
	// Releases a node index from the pool
	void RelaseNodeIndex(int32_t index);

	//-------------------------------------------------------------------------
	// Member Variables

	// m_nextindex
	//
	// Next sequential node index value
	std::atomic<int32_t> m_nextinode = 0;

	// m_spentindexes
	//
	// Queue used to recycle node indexes
	Concurrency::concurrent_queue<int32_t> m_spentinodes;

	// m_rootnode
	//
	// The root node of the file system
	std::shared_ptr<Node> m_rootnode;

	// m_extents
	//
	// TODO: I want this to be a vector<> of multiple extents
	// to allow for a smaller amount of memory to be reserved
	// than the original maximum
	std::unique_ptr<Extent> m_tempextent;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RAMFILESYSTEM_H_