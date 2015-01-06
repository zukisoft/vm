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

#ifndef __PROCESSSECTION_H_
#define __PROCESSSECTION_H_
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
// ProcessSection
//
// Implements a mapped virtual memory section

class ProcessSection
{
public:

	// Destructor
	//
	~ProcessSection();

	// less-than comparison operator
	//
	bool operator <(const ProcessSection& rhs) const { return BaseAddress < rhs.BaseAddress; }

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
	void Allocate(void* address, size_t length, uint32_t protection);

	// ChangeMode
	//
	// Alters the protection behavior mode of the section
	void ChangeMode(Mode mode);

	// Create (static)
	//
	// Creates a new virtual memory section and mapping
	static std::unique_ptr<ProcessSection> Create(HANDLE process, size_t length) { return Create(process, nullptr, length, Mode::Private, 0); }
	static std::unique_ptr<ProcessSection> Create(HANDLE process, size_t length, Mode mode) { return Create(process, nullptr, length, mode, 0); }
	static std::unique_ptr<ProcessSection> Create(HANDLE process, size_t length, uint32_t flags) { return Create(process, nullptr, length, Mode::Private, flags); }
	static std::unique_ptr<ProcessSection> Create(HANDLE process, size_t length, Mode mode, uint32_t flags) { return Create(process, nullptr, length, mode, flags); }
	static std::unique_ptr<ProcessSection> Create(HANDLE process, void* address, size_t length, Mode mode) { return Create(process, address, length, mode, 0); }
	static std::unique_ptr<ProcessSection> Create(HANDLE process, void* address, size_t length, Mode mode, uint32_t flags);

	// FromSection (static)
	//
	// Creates a duplicate of this memory section for another process
	static std::unique_ptr<ProcessSection> FromSection(const std::unique_ptr<ProcessSection>& section, HANDLE process, Mode mode);
	
	// Protect
	//
	// Changes the protection flags for pages within the virtual memory section
	void Protect(void* address, size_t length, uint32_t protection);

	// Release
	//
	// Releases pages within the virtual memory section
	void Release(void* address, size_t length);

	//-------------------------------------------------------------------------
	// Fields

	// BaseAddress
	//
	// The base address of the mapped section
	void* const	BaseAddress;

	// Length
	//
	// The length of the mapped section
	const size_t Length;

	//-------------------------------------------------------------------------
	// Properties

	// Empty
	//
	// Determines if the entire section is empty
	__declspec(property(get=getEmpty)) bool Empty;
	bool getEmpty(void) { return m_allocmap.Empty; }

private:

	ProcessSection(const ProcessSection&)=delete;
	ProcessSection& operator=(const ProcessSection&)=delete;

	// Instance Constructors
	//
	ProcessSection(HANDLE process, HANDLE section, void* baseaddress, size_t length, Mode mode);
	ProcessSection(HANDLE process, HANDLE section, void* const baseaddress, size_t length, Mode mode, const Bitmap& bitmap);

	friend std::unique_ptr<ProcessSection> std::make_unique<ProcessSection, HANDLE&, HANDLE&, void*&, size_t&, Mode&>(HANDLE&, HANDLE&, void*&, size_t&, Mode&);
	friend std::unique_ptr<ProcessSection> std::make_unique<ProcessSection, HANDLE&, HANDLE&, void*&, size_t&, Mode&, Bitmap&>(HANDLE&, HANDLE&, void*&, size_t&, Mode&, Bitmap&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// Clone
	//
	// Clones this process memory section into another process
	static std::unique_ptr<ProcessSection> Clone(const std::unique_ptr<ProcessSection>& rhs, HANDLE process, Mode mode);

	// Duplicate
	//
	// Creates a duplicate (Mode::Private) of this process memory section
	static std::unique_ptr<ProcessSection> Duplicate(const std::unique_ptr<ProcessSection>& rhs, HANDLE process);

	//-------------------------------------------------------------------------
	// Member Variables

	const HANDLE			m_process;		// The target process handle
	const HANDLE			m_section;		// The section object handle
	Mode					m_mode;			// The current section mode
	Bitmap					m_allocmap;		// Page allocation bitmap
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESSSECTION_H_
