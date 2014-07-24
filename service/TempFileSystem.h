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

#ifndef __TEMPFILESYSTEM_H_
#define __TEMPFILESYSTEM_H_
#pragma once

#include <atomic>
#include <concurrent_queue.h>
#include "FileSystem.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)		// inline specifier cannot be used (friend)

//-----------------------------------------------------------------------------
// Class RamFileSystem
//
// Implements a temporary, ram-based file system

//
// FileSystem --> this is the superblock

class TempFileSystem : public FileSystem
{
public:

	TempFileSystem()=default;
	~TempFileSystem()=default;

	// BlockSize (FileSystem)
	//
	// Gets the file system block size
	__declspec(property(get=getBlockSize)) size_t BlockSize;
	virtual size_t getBlockSize(void) const { return Chunk::BlockSize; }

	// Name (FileSystem)
	//
	// Gets the name of the file system
	__declspec(property(get=getName)) const char_t* Name;
	virtual const char_t* getName(void) const { return s_fsname; }

private:

	TempFileSystem(const TempFileSystem&)=delete;
	TempFileSystem& operator=(const TempFileSystem&)=delete;

	// Chunk
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

	// Node
	//
	// todo: words
	class Node : public FileSystem::Node
	{
	public:

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;
	};

	// Directory
	//
	// todo: words
	class Directory : public FileSystem::Directory
	{
	public:

	private:

		Directory(const Directory&)=delete;
		Directory& operator=(const Directory&)=delete;
	};

	// m_nextindex
	//
	// Next sequential node index value
	std::atomic<int32_t> m_nextindex = 0;

	// m_spentindexes
	//
	// Priority queue used to recycle node indexes; lower values will be used first
	Concurrency::concurrent_queue<int32_t> m_spentindexes;

	// s_fsname
	//
	// Name of the file system; returned through getName()
	static const char_t* s_fsname;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TEMPFILESYSTEM_H_
