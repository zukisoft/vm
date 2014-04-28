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

#ifndef __MAPPEDFILEVIEW_H_
#define __MAPPEDFILEVIEW_H_
#pragma once

#include <memory>
#include "Exception.h"
#include "MappedFile.h"
#include "Win32Exception.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// MappedFileView
//
// Creates a view of a memory-mapped file

class MappedFileView
{
public:

	// Destructor
	//
	~MappedFileView();

	//-------------------------------------------------------------------------
	// Member Functions

	// Create
	//
	// Creates a view of the specified memory mapped file
	static std::unique_ptr<MappedFileView> Create(const std::unique_ptr<MappedFile>& mapping) 
		{ return std::unique_ptr<MappedFileView>(new MappedFileView(mapping, FILE_MAP_READ, 0, 0)); }
	static std::unique_ptr<MappedFileView> Create(std::unique_ptr<MappedFile>&& mapping)
		{ return Create(std::forward<const std::unique_ptr<MappedFile>&>(mapping)); }

	static std::unique_ptr<MappedFileView> Create(const std::unique_ptr<MappedFile>& mapping, uint32_t access) 
		{ return std::unique_ptr<MappedFileView>(new MappedFileView(mapping, access, 0, 0)); }
	static std::unique_ptr<MappedFileView> Create(std::unique_ptr<MappedFile>&& mapping, uint32_t access)
		{ return Create(std::forward<const std::unique_ptr<MappedFile>&>(mapping), access); }

	static std::unique_ptr<MappedFileView> Create(const std::unique_ptr<MappedFile>& mapping, uint32_t access, size_t offset)
		{ return std::unique_ptr<MappedFileView>(new MappedFileView(mapping, access, offset, 0)); }
	static std::unique_ptr<MappedFileView> Create(std::unique_ptr<MappedFile>&& mapping, uint32_t access, size_t offset)
		{ return Create(std::forward<const std::unique_ptr<MappedFile>&>(mapping), access, offset); }

	static std::unique_ptr<MappedFileView> Create(const std::unique_ptr<MappedFile>& mapping, uint32_t access, size_t offset, size_t length)
		{ return std::unique_ptr<MappedFileView>(new MappedFileView(mapping, access, offset, length)); }
	static std::unique_ptr<MappedFileView> Create(std::unique_ptr<MappedFile>&& mapping, uint32_t access, size_t offset, size_t length)
		{ return Create(std::forward<const std::unique_ptr<MappedFile>&>(mapping), access, offset, length); }

	//-------------------------------------------------------------------------
	// Properties

	// Length
	//
	// Gets the length of the memory mapped file
	__declspec(property(get=getLength)) size_t Length;
	size_t getLength(void) const { return m_length; }

	// Pointer
	//
	// Gets the base pointer for the created memory mapping
	__declspec(property(get=getPointer)) void* Pointer;
	void* getPointer(void) const { return m_view; }

private:

	MappedFileView(const MappedFileView&)=delete;
	MappedFileView& operator=(const MappedFileView&)=delete;

	// Instance Constructor
	//
	MappedFileView(const std::unique_ptr<MappedFile>& mapping, uint32_t access, size_t offset, size_t length);

	//-------------------------------------------------------------------------
	// Member Variables

	void*							m_view;			// Base pointer of the view
	size_t							m_length;		// Length of the view
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MAPPEDFILEVIEW_H_
