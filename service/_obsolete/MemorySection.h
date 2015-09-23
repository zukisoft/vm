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

#ifndef __MEMORYSECTION_H_
#define __MEMORYSECTION_H_
#pragma once

#include <memory>
#include "Bitmap.h"
#include "NtApi.h"
#include "StructuredException.h"
#include "SystemInformation.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// MemorySection
//
// Implements a mapped virtual memory section

class MemorySection
{
public:

	// Destructor
	//
	~MemorySection();

	//-------------------------------------------------------------------------
	// Type Declarations

	// SectionMode
	//
	// Defines the page protection behavior of the section
	enum class SectionMode {

		Private			= 0,		// Mapping is private to the process
		Shared,						// Mapping is shared with another process
		CopyOnWrite,				// Mapping is set for copy-on-write access
	};

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates pages within the virtual memory section
	void* Allocate(void* address, size_t length, uint32_t protection);

	// Clone
	//
	// Clones this section into another process
	std::unique_ptr<MemorySection> Clone(HANDLE process);

	// Create (static)
	//
	// Creates a new virtual memory section and mapping
	static std::unique_ptr<MemorySection> Create(HANDLE target, size_t length) { return Create(target, nullptr, length, SectionMode::Private, 0); }
	static std::unique_ptr<MemorySection> Create(HANDLE target, size_t length, SectionMode mode) { return Create(target, nullptr, length, mode, 0); }
	static std::unique_ptr<MemorySection> Create(HANDLE target, size_t length, uint32_t flags) { return Create(target, nullptr, length, SectionMode::Private, flags); }
	static std::unique_ptr<MemorySection> Create(HANDLE target, size_t length, SectionMode mode, uint32_t flags) { return Create(target, nullptr, length, mode, flags); }
	static std::unique_ptr<MemorySection> Create(HANDLE target, void* address, size_t length) { return Create(target, address, length, SectionMode::Private, 0); }
	static std::unique_ptr<MemorySection> Create(HANDLE target, void* address, size_t length, SectionMode mode) { return Create(target, address, length, mode, 0); }
	static std::unique_ptr<MemorySection> Create(HANDLE target, void* address, size_t length, SectionMode mode, uint32_t flags);

	// Duplicate
	//
	// Duplicates this section into another process
	std::unique_ptr<MemorySection> Duplicate(HANDLE process);
	
	// Protect
	//
	// Changes the protection flags for pages within the virtual memory section
	void Protect(void* address, size_t length, uint32_t protection);

	// Release
	//
	// Releases pages within the virtual memory section
	void Release(void* address, size_t length);

	//-------------------------------------------------------------------------
	// Properties

	// BaseAddress
	//
	// The base address of the mapped section
	__declspec(property(get=getBaseAddress)) void* const BaseAddress;
	void* const getBaseAddress(void) const;

	// Empty
	//
	// Determines if the entire section is empty
	__declspec(property(get=getEmpty)) bool Empty;
	bool getEmpty(void) const;

	// Length
	//
	// Length of the mapped Section
	__declspec(property(get=getLength)) size_t Length;
	size_t getLength(void) const;

	// Mode
	//
	// Gets the page protection mode for this section
	__declspec(property(get=getMode)) SectionMode Mode;
	SectionMode getMode(void) const;

private:

	MemorySection(const MemorySection&)=delete;
	MemorySection& operator=(const MemorySection&)=delete;

	// Instance Constructors
	//
	MemorySection(HANDLE process, HANDLE section, void* baseaddress, size_t length, SectionMode mode);
	MemorySection(HANDLE process, HANDLE section, void* const baseaddress, size_t length, SectionMode mode, const Bitmap& bitmap);

	friend std::unique_ptr<MemorySection> std::make_unique<MemorySection, HANDLE&, HANDLE&, void*&, size_t&, SectionMode&>(HANDLE&, HANDLE&, void*&, size_t&, SectionMode&);
	friend std::unique_ptr<MemorySection> std::make_unique<MemorySection, HANDLE&, HANDLE&, void*&, size_t&, SectionMode&, Bitmap&>(HANDLE&, HANDLE&, void*&, size_t&, SectionMode&, Bitmap&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ChangeMode
	//
	// Alters the protection behavior mode of the section
	void ChangeMode(SectionMode mode);

	//-------------------------------------------------------------------------
	// Member Variables

	HANDLE					m_process;			// The target process handle
	HANDLE					m_section;			// The section object handle
	void*					m_address;			// Mapped section base address
	size_t					m_length;			// Length of the mapped section
	SectionMode				m_mode;				// The current section mode
	Bitmap					m_allocmap;			// Page allocation bitmap
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MEMORYSECTION_H_
