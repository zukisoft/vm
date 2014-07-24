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

#include "stdafx.h"
#include "TempFileSystem.h"

#pragma warning(push, 4)

// TempFileSystem::s_fsname
//
// Defines the file system name; returned through getName()
const char_t* TempFileSystem::s_fsname = "tmpfs";

// TempFileSystem::Chunk::BlockSize
//
// Initialized to the system memory page size
const size_t TempFileSystem::Chunk::BlockSize = []() -> size_t {

	SYSTEM_INFO info;
	GetNativeSystemInfo(&info);
	return info.dwPageSize;
}();

//-----------------------------------------------------------------------------
// TempFileSystem::Chunk Destructor

TempFileSystem::Chunk::~Chunk()
{
	// Release the entire chunk of virtual memory on destruction
	if(m_base) VirtualFree(reinterpret_cast<void*>(m_base), 0, MEM_RELEASE);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Chunk::AllocateBlock
//
// Allocates a block of data from the chunk
//
// Arguments:
//
//	NONE

int16_t TempFileSystem::Chunk::AllocateBlock(void)
{
	// The other version of AllocateBlock() that returns the pointer
	// to the data is undoubtedly going to be more useful, implementation
	// has been moved into that function ...

	int16_t index;
	AllocateBlock(index);
	return index;
}

//-----------------------------------------------------------------------------
// TempFileSystem::Chunk::AllocateBlock
//
// Allocates a block of data from the chunk and provides the base pointer
//
// Arguments:
//
//	index		- Reference to receive the allocated block index

void* TempFileSystem::Chunk::AllocateBlock(int16_t& index)
{
	// Try to grab a spent index first, otherwise grab a new one
	if(!m_spentblocks.try_pop(index)) index = m_nextblock++;

	// Check that the index is within the boundaries of this chunk
	if((index < 0) || (index >= m_total)) throw Win32Exception(ERROR_INVALID_ADDRESS);

	// Calculate the address of the block being committed and validate it
	void* address = reinterpret_cast<void*>(m_base + (index * BlockSize));
	_ASSERTE(ValidateBlockState(address, MEM_RESERVE));

	// Commit the page with READWRITE access only, no EXECUTE here
	if(!VirtualAlloc(address, BlockSize, MEM_COMMIT, PAGE_READWRITE)) throw Win32Exception();

	return address;
}

//-----------------------------------------------------------------------------
// TempFileSystem::Chunk::Create (static)
//
// Creates a Chunk instance
//
// Arguments:
//
//	blocks		- Number of blocks to reserve for this chunk

std::unique_ptr<TempFileSystem::Chunk> TempFileSystem::Chunk::Create(int16_t blocks)
{
	// Attempt to reserve a contiguous address space for the chunk
	void* base = VirtualAlloc(nullptr, blocks * BlockSize, MEM_RESERVE, PAGE_NOACCESS);
	if(base == nullptr) throw Win32Exception();

	return std::make_unique<Chunk>(base, blocks);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Chunk::getBlock
//
// Gets the base pointer for an allocated (committed) block
//
// Arguments:
//
//	index		- Allocated block index

void* TempFileSystem::Chunk::getBlock(int16_t index) const
{
	// Check that the index is within the boundaries of this chunk
	if((index < 0) || (index > m_total)) throw Win32Exception(ERROR_INVALID_ADDRESS);

	// Calculate the address of the block being accessed and validate it
	void* address = reinterpret_cast<void*>(m_base + (index * BlockSize));
	_ASSERTE(ValidateBlockState(address, MEM_COMMIT));

	return address;
}

//-----------------------------------------------------------------------------
// TempFileSystem::Chunk::ReleaseBlock
//
// Releases a block from the chunk and marks it as available for reuse
//
// Arguments:
//
//	index		- Allocated block index

void TempFileSystem::Chunk::ReleaseBlock(int16_t index)
{
	// Check that the index is within the boundaries of this chunk
	if((index < 0) || (index > m_total)) throw Win32Exception(ERROR_INVALID_ADDRESS);

	// Calculate the address of the block being released and validate it
	void* address = reinterpret_cast<void*>(m_base + (index * BlockSize));
	_ASSERTE(ValidateBlockState(address, MEM_COMMIT));

	// Decommit the block from virtual memory and add the index to the spent blocks queue
	if(!VirtualFree(address, BlockSize, MEM_DECOMMIT)) throw Win32Exception();
	m_spentblocks.push(index);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Chunk::ValidateBlockState (private)
//
// Verifies that a block address is in an expected state (committed/decommitted)
//
// Arguments:
//
//	address		- Block address to validate
//	state		- Expected block state

bool TempFileSystem::Chunk::ValidateBlockState(void* address, DWORD state) const
{
	// Query the information about the specified address
	MEMORY_BASIC_INFORMATION meminfo;
	VirtualQuery(address, &meminfo, sizeof(MEMORY_BASIC_INFORMATION));

	// Check the state of the page against what the caller provided
	return (meminfo.State == state);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
