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
#include <vector>
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

	// Class Chunk
	//
	// Implements a chunk of virtual memory, divided up into blocks of data
	// based on the system page size.  Allocation/release of individual blocks 
	// is atomic, however access to the block data must be properly serialized 
	// externally to this class.
	//
	// The number of blocks is limited to 32768 (INT16_MAX) to allow for a single 
	// 32-bit variable to hold a CHUNK:BLOCK memory index in the outer class, this
	// caps a single chunk to 128MiB with standard 4K memory pages
	class Chunk
	{
	public:

		// Destructor
		~Chunk();

		// AllocateBlock
		//
		// Allocates (commits) a single block from this chunk
		int16_t AllocateBlock(void);
		void* AllocateBlock(int16_t& index);

		// Create (static)
		//
		// Create a new Chunk with the specified number of blocks
		static std::unique_ptr<Chunk> Create(int16_t blocks);

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

		Chunk(const Chunk&)=delete;
		Chunk& operator=(const Chunk&)=delete;

		// Instance Constructor
		Chunk(void* base, int16_t blocks) : m_base(uintptr_t(base)), m_total(blocks) {}
		friend std::unique_ptr<Chunk> std::make_unique<Chunk, void*&, int16_t&>(void*&, int16_t&);

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

	// Class Node
	//
	// Node implementation for the file system
	class Node : public FileSystem::Node
	{
	public:

		// Constructor / Destructor
		//
		// TODO: needs type, mode and uid/gid arguments
		Node(RamFileSystem& fs);
		virtual ~Node();

		// (FileSystem::Node)

		//virtual FileSystem::AliasPtr Create(const std::shared_ptr<FileSystem::Alias>& parent, uapi::mode_t mode);

		virtual uint32_t getIndex(void) const { return m_index; }

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

		uint32_t m_index;

		// m_blocks
		//
		// Vector of blocks allocated for this node
		std::vector<uint32_t> m_blocks;

		// m_fs
		//
		// Reference to the parent file system
		RamFileSystem& m_fs;

		// m_lock
		//
		// Block data read/write access lock
		std::recursive_mutex m_lock;
	};

	// Alias
	//
	// Implementation of FileSystem::Alias for this file system
	class Alias : public FileSystem::Alias
	{
	public:

		// Constructors
		//
		Alias(RamFileSystem& fs);
		Alias(RamFileSystem& fs, const std::shared_ptr<FileSystem::Alias>& parent, const char_t* name);

		// Destructor
		//
		~Alias()=default;

		// AttachNode
		//
		// Attaches a Node instance to this Alias instance
		virtual void AttachNode(const std::shared_ptr<FileSystem::Node>& node);

		// DetachNode
		//
		// Detaches the node instance from this Alias
		virtual void DetachNode(void) {}

	private:

		Alias(const Alias&)=delete;
		Alias& operator=(const Alias&)=delete;

		// m_children
		//
		// Collection of Aliases that are children of this alias,
		// are not necessarily from this file system
		std::vector<std::weak_ptr<FileSystem::Alias>> m_children;

		// m_fs
		//
		// Reference to the parent file system instance
		RamFileSystem& m_fs;

		// m_name
		//
		// The alias name
		std::string m_name;

		// m_node
		//
		// The underlying Node instance this alias points to
		std::weak_ptr<Node> m_node;

		// m_parent
		//
		// The parent Alias for this alias
		std::shared_ptr<FileSystem::Alias> m_parent;
	};

	class DirectoryEntry : public FileSystem::DirectoryEntry
	{
	public:

		// Constructor / Destructor
		//
		DirectoryEntry(RamFileSystem& fs, const std::shared_ptr<Node>& node);
		virtual ~DirectoryEntry()=default;

	protected:

		RamFileSystem& m_fs;
		std::shared_ptr<Node> m_node;

	private:

		DirectoryEntry(const DirectoryEntry&)=delete;
		DirectoryEntry& operator=(const DirectoryEntry&)=delete;
	};

	// this represents a view of the node by a process
	class File : public FileSystem::File
	{
	public:

		// Constructor / Destructor
		//
		File(RamFileSystem& fs, const std::shared_ptr<Node>& node);
		virtual ~File()=default;

	private:

		File(const File&)=delete;
		File& operator=(const File&)=delete;

		RamFileSystem& m_fs;
		std::shared_ptr<Node> m_node;
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

	// m_root
	//
	// The file system root object
	std::shared_ptr<Node> m_root;

	// m_chunks
	//
	// TODO: I want this to be a vector<> of multiple chunks
	// to allow for a smaller amount of memory to be reserved
	// than the original maximum
	std::unique_ptr<Chunk> m_tempchunk;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RAMFILESYSTEM_H_