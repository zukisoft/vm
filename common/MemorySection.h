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

	// Mode
	//
	// Defines the protection behavior of the section
	enum class Mode {

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

	// ChangeMode
	//
	// Alters the protection behavior mode of the section
	void ChangeMode(Mode mode);

	// Create (static)
	//
	// Creates a new virtual memory section and mapping
	static std::unique_ptr<MemorySection> Create(HANDLE process, size_t length) { return Create(process, nullptr, length, Mode::Private, 0); }
	static std::unique_ptr<MemorySection> Create(HANDLE process, size_t length, Mode mode) { return Create(process, nullptr, length, mode, 0); }
	static std::unique_ptr<MemorySection> Create(HANDLE process, size_t length, uint32_t flags) { return Create(process, nullptr, length, Mode::Private, flags); }
	static std::unique_ptr<MemorySection> Create(HANDLE process, size_t length, Mode mode, uint32_t flags) { return Create(process, nullptr, length, mode, flags); }
	static std::unique_ptr<MemorySection> Create(HANDLE process, void* address, size_t length) { return Create(process, address, length, Mode::Private, 0); }
	static std::unique_ptr<MemorySection> Create(HANDLE process, void* address, size_t length, Mode mode) { return Create(process, address, length, mode, 0); }
	static std::unique_ptr<MemorySection> Create(HANDLE process, void* address, size_t length, Mode mode, uint32_t flags);

	// FromSection (static)
	//
	// Creates a duplicate of this memory section for another process
	static std::unique_ptr<MemorySection> FromSection(const std::unique_ptr<MemorySection>& section, HANDLE process, Mode mode);
	
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
	void* const getBaseAddress(void) const { return m_address; }

	// Empty
	//
	// Determines if the entire section is empty
	__declspec(property(get=getEmpty)) bool Empty;
	bool getEmpty(void) const { return m_allocmap.Empty; }

	// Length
	//
	// Length of the mapped Section
	__declspec(property(get=getLength)) size_t Length;
	size_t getLength(void) const { return m_length; }

private:

	MemorySection(const MemorySection&)=delete;
	MemorySection& operator=(const MemorySection&)=delete;

	// Instance Constructors
	//
	MemorySection(HANDLE process, HANDLE section, void* baseaddress, size_t length, Mode mode);
	MemorySection(HANDLE process, HANDLE section, void* const baseaddress, size_t length, Mode mode, const Bitmap& bitmap);

	friend std::unique_ptr<MemorySection> std::make_unique<MemorySection, HANDLE&, HANDLE&, void*&, size_t&, Mode&>(HANDLE&, HANDLE&, void*&, size_t&, Mode&);
	friend std::unique_ptr<MemorySection> std::make_unique<MemorySection, HANDLE&, HANDLE&, void*&, size_t&, Mode&, Bitmap&>(HANDLE&, HANDLE&, void*&, size_t&, Mode&, Bitmap&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// Clone
	//
	// Clones this process memory section into another process
	static std::unique_ptr<MemorySection> Clone(const std::unique_ptr<MemorySection>& rhs, HANDLE process, Mode mode);

	// Duplicate
	//
	// Creates a duplicate (Mode::Private) of this process memory section
	static std::unique_ptr<MemorySection> Duplicate(const std::unique_ptr<MemorySection>& rhs, HANDLE process);

	//-------------------------------------------------------------------------
	// Member Variables

	HANDLE					m_process;			// The target process handle
	HANDLE					m_section;			// The section object handle
	void*					m_address;			// Mapped section base address
	size_t					m_length;			// Length of the mapped section
	Mode					m_mode;				// The current section mode
	Bitmap					m_allocmap;			// Page allocation bitmap
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MEMORYSECTION_H_
