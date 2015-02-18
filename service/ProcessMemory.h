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
#include <concrt.h>
#include <linux/mman.h>
#include "LinuxException.h"
#include "MemorySection.h"
#include "NativeHandle.h"
#include "NtApi.h"
#include "StructuredException.h"
#include "SystemInformation.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

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

	// DuplicationMode
	//
	// Defines the address space duplication mode for FromProcessMemory(). Values
	// are analogous to the MemorySection protection mode flags
	enum class DuplicationMode {

		Clone			= static_cast<int>(MemorySection::Mode::CopyOnWrite),
		Share			= static_cast<int>(MemorySection::Mode::Shared),
		Duplicate		= static_cast<int>(MemorySection::Mode::Private),
	};

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates virtual memory
	const void* Allocate(size_t length, int prot);
	const void* Allocate(const void* address, size_t length, int prot);

	// Clear
	//
	// Removes all virtual memory allocations 
	void Clear(void);

	// Create (static)
	//
	// Creates a new empty process virtual address space
	static std::unique_ptr<ProcessMemory> Create(const std::shared_ptr<NativeHandle>& process);

	// FromProcessMemory (static)
	//
	// Duplicates the virtual address space from one process into another
	static std::unique_ptr<ProcessMemory> FromProcessMemory(const std::shared_ptr<NativeHandle>& process, const std::unique_ptr<ProcessMemory>& existing, DuplicationMode mode);

	// Protect
	//
	// Sets the memory protection flags for a virtual memory region
	void Protect(const void* address, size_t length, int prot);

	// Read
	//
	// Reads data from the process address space
	size_t Read(const void* address, void* buffer, size_t length);

	// Release
	//
	// Releases virtual memory
	void Release(const void* address, size_t length);

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
	using section_lock_t = Concurrency::reader_writer_lock;
	
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
	// Member Variables

	std::shared_ptr<NativeHandle>	m_process;			// Native process handle
	section_lock_t					m_sectionlock;		// Section collection lock
	section_vector_t				m_sections;			// Memory section collection
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESSMEMORY_H_
