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

#ifndef __MEMORYSECTION_H_
#define __MEMORYSECTION_H_
#pragma once

#include <memory>
#include "StructuredException.h"
#include "SystemInformation.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// MemorySection
//
// Implements a virtual memory section that can be cloned between processes

class MemorySection
{
public:

	// Destructor
	//
	~MemorySection();

	//-------------------------------------------------------------------------
	// Type Declarations

	// CloneMode
	//
	// Indicates the mode used when cloning this section into another process
	enum class CloneMode {

		Shared,					// Clone as a shared memory section
		SharedCopyOnWrite,		// Clone as a copy-on-write shared memory section
		Duplicate				// Clone as a duplicate memory section
	};

	//-------------------------------------------------------------------------
	// Member Functions

	// Clone
	//
	// Clones this section into another process
	std::unique_ptr<MemorySection> Clone(HANDLE process, CloneMode mode);

	// Commit
	//
	// Commits page(s) within the section
	void Commit(void* address, size_t length, uint32_t protect);

	// Decommit
	//
	// Decommits page(s) within the section
	void Decommit(void* address, size_t length);

	// Protect
	//
	// Applies protection flags to page(s) within the section
	uint32_t Protect(void* address, size_t length, uint32_t protect);	
	
	// Reserve (static)
	//
	// Creates and reserves a new section
	static std::unique_ptr<MemorySection> Reserve(HANDLE process, size_t length) { return Reserve(process, nullptr, length, 0); }
	static std::unique_ptr<MemorySection> Reserve(HANDLE process, size_t length, uint32_t mapflags) { return Reserve(process, nullptr, length, mapflags); }
	static std::unique_ptr<MemorySection> Reserve(HANDLE process, void* address, size_t length) { return Reserve(process, address, length, 0); }
	static std::unique_ptr<MemorySection> Reserve(HANDLE process, void* address, size_t length, uint32_t mapflags);

	//-------------------------------------------------------------------------
	// Properties

	// BaseAddress
	//
	// Gets the base address for the mapped section
	__declspec(property(get=getBaseAddress)) void* BaseAddress;
	void* getBaseAddress(void) const { return m_address; }

	// Length
	//
	// Gets the length of the mapped section
	__declspec(property(get=getLength)) size_t Length;
	size_t getLength(void) const { return m_length; }

private:

	MemorySection(const MemorySection&)=delete;
	MemorySection& operator=(const MemorySection&)=delete;

	// Instance Constructor
	//
	MemorySection(HANDLE process, HANDLE section, void* address, size_t length) : m_process(process), m_section(section), m_address(address), m_length(length) {}
	friend std::unique_ptr<MemorySection> std::make_unique<MemorySection, HANDLE&, HANDLE&, void*&, size_t&>(HANDLE&, HANDLE&, void*&, size_t&);

	// Duplicate
	//
	// Duplicates the section into another process
	std::unique_ptr<MemorySection> Duplicate(HANDLE process);
	
	//-------------------------------------------------------------------------
	// Private Type Declarations

	// SECTION_INHERIT
	//
	// Section inheritance flags for NtMapViewOfSection
	using SECTION_INHERIT = int;
	static const SECTION_INHERIT ViewShare = 1;
	static const SECTION_INHERIT ViewUnmap = 2;

	// DUPLICATE_SAME_ATTRIBUTES
	//
	// NTAPI constant not defined in the standard Win32 user-mode headers
	static const int DUPLICATE_SAME_ATTRIBUTES = 0x04;

	// STATUS_SUCCESS
	//
	// NTAPI constant not defined in the standard Win32 user-mode headers
	static const NTSTATUS STATUS_SUCCESS = 0;

	// NTAPI Functions
	//
	using NtAllocateVirtualMemoryFunc	= std::function<NTSTATUS(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG)>;
	using NtCreateSectionFunc			= std::function<NTSTATUS(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE)>;
	using NtDuplicateObjectFunc			= std::function<NTSTATUS(HANDLE, HANDLE, HANDLE, PHANDLE, ACCESS_MASK, ULONG, ULONG)>;
	using NtFreeVirtualMemoryFunc		= std::function<NTSTATUS(HANDLE, PVOID, PSIZE_T, ULONG)>;
	using NtProtectVirtualMemoryFunc	= std::function<NTSTATUS(HANDLE, PVOID*, SIZE_T*, ULONG, PULONG)>;
	using NtMapViewOfSectionFunc		= std::function<NTSTATUS(HANDLE, HANDLE, PVOID, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, SECTION_INHERIT, ULONG, ULONG)>;
	using NtUnmapViewOfSectionFunc		= std::function<NTSTATUS(HANDLE, PVOID)>;
	using NtCloseFunc					= std::function<NTSTATUS(HANDLE)>;

	//-------------------------------------------------------------------------
	// Member Variables

	HANDLE						m_process;			// Process handle (not owned by this class)
	HANDLE						m_section;			// Section handle (owned by this class)
	void*						m_address;			// Base address of the mapped section
	size_t						m_length;			// Length of the mapped section

	// NTAPI
	//
	static NtAllocateVirtualMemoryFunc	NtAllocateVirtualMemory;
	static NtCloseFunc					NtClose;
	static NtCreateSectionFunc			NtCreateSection;
	static HANDLE						NtCurrentProcess;
	static NtDuplicateObjectFunc		NtDuplicateObject;
	static NtFreeVirtualMemoryFunc		NtFreeVirtualMemory;
	static NtMapViewOfSectionFunc		NtMapViewOfSection;
	static NtProtectVirtualMemoryFunc	NtProtectVirtualMemory;
	static NtUnmapViewOfSectionFunc		NtUnmapViewOfSection;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MEMORYSECTION_H_
