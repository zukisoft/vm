//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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

#ifndef __NATIVEPROCESS_H_
#define __NATIVEPROCESS_H_
#pragma once

#include <set>
#include <unordered_map>
#include "Architecture.h"
#include "Bitmap.h"
#include "ProcessMemory.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Class NativeProcess
//
// Owns a native operating system process handle and abstracts the operations that
// can be performed against that process.
//
// Memory within the native process is managed by mapped section objects so that they
// can be shared among multiple processes as necessary.  Limitations of pagefile
// backed sections are similar to Win32 file mappings -- you can create them as 
// reservations and subsequently commit individual pages, but you cannot decommit
// them again, you can only release the entire section.
//
// Due to these limitations, when a section is created it is implicitly committed into
// the process' address space, but given PAGE_NOACCESS protection flags to prevent access
// until they are soft-allocated.  Soft allocation involves changing those protection
// flags to whatever the caller wants and marking which pages are now available in a
// bitmap created for each section.  Since pages cannot be decommitted, a soft release
// operation is also used, that merely resets the protection back to PAGE_NOACCESS (note
// that the contents are not cleared).  Only when an entire section has been soft-
// released will it be removed from the collection and formally deallocated.

class NativeProcess : public ProcessMemory
{
public:

	// Instance Constructor
	//
	NativeProcess(enum class Architecture architecture, HANDLE process, DWORD processid);

	// Destructor
	//
	~NativeProcess();

	//-------------------------------------------------------------------------
	// Member Functions

	// Resume
	//
	// Resumes the process
	void Resume(void) const;

	// Suspend
	//
	// Suspends the process
	void Suspend(void) const;
	
	// Terminate
	//
	// Terminates the native process
	void Terminate(uint16_t exitcode) const;
	void Terminate(uint16_t exitcode, bool wait) const;
	
	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the (actual) architecture of the native process
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	enum class Architecture getArchitecture(void) const;

	// ExitCode
	//
	// Gets the exit code from the native process
	__declspec(property(get=getExitCode)) DWORD ExitCode;
	DWORD getExitCode(void) const;

	// ProcessHandle
	//
	// Gets the native process handle
	__declspec(property(get=getProcessHandle)) HANDLE ProcessHandle;
	HANDLE getProcessHandle(void) const;

	// ProcessId
	//
	// Gets the native process identifier
	__declspec(property(get=getProcessId)) DWORD ProcessId;
	DWORD getProcessId(void) const;

	//-------------------------------------------------------------------------
	// ProcessMemory Implementation

	// AllocateMemory
	//
	// Allocates a region of virtual memory
	virtual uintptr_t AllocateMemory(size_t length, ProcessMemory::Protection protection);
	virtual uintptr_t AllocateMemory(size_t length, ProcessMemory::Protection protection, ProcessMemory::AllocationFlags flags);
	virtual uintptr_t AllocateMemory(uintptr_t address, size_t length, ProcessMemory::Protection protection);

	// LockMemory
	//
	// Attempts to lock a region into physical memory
	virtual void LockMemory(uintptr_t address, size_t length) const;

	// MapMemory
	//
	// Maps a virtual memory region into the calling process
	virtual void* MapMemory(uintptr_t address, size_t length, ProcessMemory::Protection protection);

	// ProtectMemory
	//
	// Sets the memory protection flags for a virtual memory region
	virtual void ProtectMemory(uintptr_t address, size_t length, ProcessMemory::Protection protection) const;

	// ReadMemory
	//
	// Reads data from a virtual memory region into the calling process
	virtual size_t ReadMemory(uintptr_t address, void* buffer, size_t length) const;

	// ReleaseMemory
	//
	// Releases a virtual memory region
	virtual void ReleaseMemory(uintptr_t address, size_t length);

	// ReserveMemory
	//
	// Reserves a virtual memory region for later allocation
	virtual uintptr_t ReserveMemory(size_t length);
	virtual uintptr_t ReserveMemory(size_t length, ProcessMemory::AllocationFlags flags);
	virtual uintptr_t ReserveMemory(uintptr_t address, size_t length);

	// UnlockMemory
	//
	// Attempts to unlock a region from physical memory
	virtual void UnlockMemory(uintptr_t address, size_t length) const;

	// UnmapMemory
	//
	// Unmaps a previously mapped memory region from the calling process
	virtual void UnmapMemory(void const* mapping);

	// WriteMemory
	//
	// Writes data into a virtual memory region from the calling process
	virtual size_t WriteMemory(uintptr_t address, void const* buffer, size_t length) const;

private:

	NativeProcess(NativeProcess const&)=delete;
	NativeProcess& operator=(NativeProcess const&)=delete;

	// localmappings_t
	//
	// Collection to track local process mappings
	using localmappings_t = std::unordered_map<void const*, std::vector<uintptr_t>>;

	// section_t
	//
	// Structure used to track a section allocation and mapping
	struct section_t
	{
		// Instance Constructor
		//
		section_t(HANDLE section, uintptr_t baseaddress, size_t length);

		// Less-than operator
		//
		bool operator <(section_t const& rhs) const;

		// Fields
		//
		HANDLE const		m_section;
		uintptr_t const		m_baseaddress;
		size_t const		m_length;
		mutable Bitmap		m_allocationmap;
	};

	// sectioniterator_t
	//
	// Callback/lambda function prototype used when iterating over sections
	using sectioniterator_t = std::function<void(section_t const& section, uintptr_t address, size_t length)>;

	// sections_t
	//
	// Collection of section_t instances
	typedef std::set<section_t> sections_t;

	//-------------------------------------------------------------------------
	// Private Member Functions
	
	// CreateSection (static)
	//
	// Creates a new memory section object and maps it to the specified address
	static section_t CreateSection(HANDLE process, uintptr_t address, size_t length, ProcessMemory::AllocationFlags flags);

	// EnsureSectionAllocation (static)
	//
	// Verifies that the specified address range is soft-allocated within a section
	static void EnsureSectionAllocation(section_t const& section, uintptr_t address, size_t length);

	// IterateRange
	//
	// Iterates across an address range and invokes the specified operation for each section
	void IterateRange(sync::reader_writer_lock::scoped_lock& lock, uintptr_t start, size_t length, sectioniterator_t operation) const;

	// ReleaseLocalMappings (static)
	//
	// Releases a vector of local address mappings
	static void ReleaseLocalMappings(HANDLE process, std::vector<uintptr_t> const& mappings);

	// ReleaseSection (static)
	//
	// Releases a memory section object created by CreateSection
	static void ReleaseSection(HANDLE process, section_t const& section);

	// ReserveRange
	//
	// Ensures that a range of address space is reserved
	void ReserveRange(sync::reader_writer_lock::scoped_lock_write& writer, uintptr_t start, size_t length);
	
	//-------------------------------------------------------------------------
	// Member Variables

	enum class Architecture	const		m_architecture;		// Native process architecture
	HANDLE const						m_process;			// Native process handle
	DWORD const							m_processid;		// Native process identifier

	// Memory Management
	//
	sections_t							m_sections;			// Allocated sections
	localmappings_t						m_localmappings;	// Local section mappings
	mutable sync::reader_writer_lock	m_sectionslock;		// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NATIVEPROCESS_H_

