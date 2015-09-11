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

#ifndef __HOST_H_
#define __HOST_H_
#pragma once

#include <memory>
#include <vector>
#include "Architecture.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

// Forward Declarations
//
class MemorySection;
class NativeProcess;

//-----------------------------------------------------------------------------
// Class Host
//
// Host provides manipulation of a native operating system process for use as a
// virtual machine process.  Process handles the higher level abstractions, this
// class implements the lower level details of those abstractions

class Host
{
public:

	// Destructor
	//
	~Host();

	// Host::MemoryProtection
	//
	// Protection flags used with memory operations
	class MemoryProtection final : public bitmask<uint32_t, LINUX_PROT_NONE | LINUX_PROT_READ | LINUX_PROT_WRITE | LINUX_PROT_EXEC | LINUX_PROT_SEM>
	{
	public:

		using bitmask::bitmask;

		//-------------------------------------------------------------------------
		// Fields

		// Atomic (static)
		//
		// Indicates that the memory region can be used for atomic operations
		static const MemoryProtection Atomic;

		// Execute (static)
		//
		// Indicates that the memory region can be executed
		static const MemoryProtection Execute;

		// None (static)
		//
		// Indicates that the memory region cannot be accessed
		static const MemoryProtection None;

		// Read (static)
		//
		// Indicates that the memory region can be read
		static const MemoryProtection Read;

		// Write (static)
		//
		// Indicates that the memory region can be written to
		static const MemoryProtection Write;
	};

	//-------------------------------------------------------------------------
	// Member Functions

	// AllocateMemory
	//
	// Allocates virtual memory
	const void* AllocateMemory(size_t length, MemoryProtection prot);
	const void* AllocateMemory(const void* address, size_t length, MemoryProtection prot);

	// ClearMemory
	//
	// Removes all virtual memory allocations 
	void ClearMemory(void);
	
	// Clone
	//
	// Clones this host instance into another NativeProcess instance
	std::unique_ptr<Host> Clone(std::unique_ptr<NativeProcess> nativeproc);

	// Create (static)
	//
	// Constructs a new Host instance from an existing NativeProcess instance
	static std::unique_ptr<Host> Create(std::unique_ptr<NativeProcess> nativeproc);

	// GuardMemory
	//
	// Sets up guard pages within a virtual memory region
	void GuardMemory(const void* address, size_t length, MemoryProtection prot) const;

	// LockMemory
	//
	// Attempts to lock a region into the process working set
	void LockMemory(const void* address, size_t length) const;

	// ProtectMemory
	//
	// Sets the memory protection flags for a virtual memory region
	void ProtectMemory(const void* address, size_t length, MemoryProtection prot) const;

	// ReadMemory
	//
	// Reads data from the process address space
	size_t ReadMemory(const void* address, void* buffer, size_t length) const;

	// ReleaseMemory
	//
	// Releases virtual memory
	void ReleaseMemory(const void* address, size_t length);

	// Terminate
	//
	// Terminates the host process
	void Terminate(uint16_t exitcode) const;
	void Terminate(uint16_t exitcode, bool wait) const;

	// UnlockMemory
	//
	// Attempts to unlock a region from the process working set
	void UnlockMemory(const void* address, size_t length) const;

	// WriteMemory
	//
	// Writes data into the process address space
	size_t WriteMemory(const void* address, const void* buffer, size_t length) const;

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture of the native host process
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	enum class Architecture getArchitecture(void) const;

private:

	Host(const Host&)=delete;
	Host& operator=(const Host&)=delete;

	// nativeproc_t
	//
	// NativeProcess unique pointer
	using nativeproc_t = std::unique_ptr<NativeProcess>;
	
	// section_vector_t
	//
	// Collection of allocated memory section instances
	using section_vector_t = std::vector<std::unique_ptr<MemorySection>>;

	// Instance Constructor
	//
	Host(nativeproc_t nativeproc, section_vector_t&& sections);
	friend std::unique_ptr<Host> std::make_unique<Host, nativeproc_t, section_vector_t>(nativeproc_t&&, section_vector_t&&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ProtectMemoryInternal
	//
	// Backing function for ProtectMemory() and GuardMemory() methods
	void ProtectMemoryInternal(const void* address, size_t length, DWORD winprot) const;

	//-------------------------------------------------------------------------
	// Member Variables

	const nativeproc_t					m_nativeproc;		// Native process instance
	section_vector_t					m_sections;			// Process memory sections
	mutable sync::reader_writer_lock	m_sectionslock;		// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOST_H_
