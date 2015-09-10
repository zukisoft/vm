//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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

#ifndef __PROCESSMEMORY_H_
#define __PROCESSMEMORY_H_
#pragma once

#include <memory>
#include <vector>
#include "MemorySection.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

// Forward Declarations
//
class MemorySection;
class NativeHandle;

//-----------------------------------------------------------------------------
// Class ProcessMemory
//
// Manages the virtual address space of a process

class ProcessMemory
{
public:

	// Destructor
	//
	~ProcessMemory()=default;

	// ProcessMemory::Protection
	//
	// Protection flags used with memory operations
	class Protection final : public bitmask<uint32_t, LINUX_PROT_NONE | LINUX_PROT_READ | LINUX_PROT_WRITE | LINUX_PROT_EXEC | LINUX_PROT_SEM>
	{
	public:

		using bitmask::bitmask;

		//-------------------------------------------------------------------------
		// Fields

		// Atomic (static)
		//
		// Indicates that the memory region can be used for atomic operations
		static const Protection Atomic;

		// Execute (static)
		//
		// Indicates that the memory region can be executed
		static const Protection Execute;

		// None (static)
		//
		// Indicates that the memory region cannot be accessed
		static const Protection None;

		// Read (static)
		//
		// Indicates that the memory region can be read
		static const Protection Read;

		// Write (static)
		//
		// Indicates that the memory region can be written to
		static const Protection Write;
	};

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates virtual memory
	const void* Allocate(size_t length, Protection prot);
	const void* Allocate(const void* address, size_t length, Protection prot);

	// Clear
	//
	// Removes all virtual memory allocations 
	void Clear(void);

	// Clone
	//
	// Clones the virtual address space from one process into another
	std::unique_ptr<ProcessMemory> Clone(const std::shared_ptr<NativeHandle>& target);

	// Create (static)
	//
	// Creates a new empty process virtual address space
	static std::unique_ptr<ProcessMemory> Create(const std::shared_ptr<NativeHandle>& process);

	// Duplicate
	//
	// Duplicates the virtual address space from one process into another
	std::unique_ptr<ProcessMemory> Duplicate(const std::shared_ptr<NativeHandle>& target);

	// Guard
	//
	// Sets up guard pages within a virtual memory region
	void Guard(const void* address, size_t length, Protection prot);

	// Lock
	//
	// Attempts to lock a region into the process working set
	void Lock(const void* address, size_t length) const;

	// Protect
	//
	// Sets the memory protection flags for a virtual memory region
	void Protect(const void* address, size_t length, Protection prot);

	// Read
	//
	// Reads data from the process address space
	size_t Read(const void* address, void* buffer, size_t length);

	// Release
	//
	// Releases virtual memory
	void Release(const void* address, size_t length);

	// Unlock
	//
	// Attempts to unlock a region from the process working set
	void Unlock(const void* address, size_t length) const;

	// Write
	//
	// Writes data into the process address space
	size_t Write(const void* address, const void* buffer, size_t length);

private:

	ProcessMemory(const ProcessMemory&)=delete;
	ProcessMemory& operator=(const ProcessMemory&)=delete;

	// section_lock_t
	//
	// Memory section collection synchronization object
	using section_lock_t = sync::reader_writer_lock;
	
	// section_vector_t
	//
	// Collection of allocated memory section instances
	using section_vector_t = std::vector<std::unique_ptr<MemorySection>>;

	// Instance Constructor
	//
	ProcessMemory(const std::shared_ptr<NativeHandle>& process, section_vector_t&& sections);
	friend std::unique_ptr<ProcessMemory> std::make_unique<ProcessMemory, const std::shared_ptr<NativeHandle>&, section_vector_t>
		(const std::shared_ptr<NativeHandle>& process, section_vector_t&& sections);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ProtectInternal
	//
	// Backing function for Protect() and Guard() methods
	void ProtectInternal(const void* address, size_t length, uint32_t winprot);

	//-------------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<NativeHandle>	m_process;			// Native process handle
	section_lock_t					m_sectionlock;		// Section collection lock
	section_vector_t				m_sections;			// Memory section collection
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESSMEMORY_H_
