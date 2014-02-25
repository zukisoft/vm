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

#ifndef __MAPPEDFILE_H_
#define __MAPPEDFILE_H_
#pragma once

#include "Exception.h"					// Include Exception declarations
#include "File.h"						// Include File declarations
#include "Win32Exception.h"				// Include Win32Exception decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// MappedFile
//
// Creates a memory-mapped file

class MappedFile
{
public:

	// Destructor
	//
	~MappedFile();

	//-------------------------------------------------------------------------
	// Overloaded Operators
	
	// HANDLE
	//
	operator HANDLE() const { return m_handle; }

	//-------------------------------------------------------------------------
	// Member Functions

	// CreateFromFile
	//
	// Creates a mapping against an existing file handle
	static MappedFile* CreateFromFile(std::shared_ptr<File>& file, DWORD protect)
		{ return new MappedFile(file, protect, 0, NULL); }

	static MappedFile* CreateFromFile(std::shared_ptr<File>& file, DWORD protect, size_t capacity)
		{ return new MappedFile(file, protect, capacity, NULL); }
	
	// CreateNew
	//
	// Creates a mapping against the system page file
	static MappedFile* CreateNew(DWORD protect, size_t capacity)
		{ return new MappedFile(s_nullptr, protect, capacity, NULL); }

	//-------------------------------------------------------------------------
	// Properties

	// Capacity
	//
	// Gets the capacity of the memory mapped file
	__declspec(property(get=getCapacity)) size_t Capacity;
	size_t getCapacity(void) const { return m_capacity; }

	// Handle
	//
	// Gets the underlying handle for the mapped file
	__declspec(property(get=getHandle)) HANDLE Handle;
	void* getHandle(void) const { return m_handle; }

private:

	MappedFile(const MappedFile&);
	MappedFile& operator=(const MappedFile&);

	// Instance Constructor
	//
	MappedFile(std::shared_ptr<File>& file, DWORD protect, size_t capacity, LPCTSTR name);

	//-------------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<File>			m_file;			// File handle
	HANDLE							m_handle;		// File mapping handle
	size_t							m_capacity;		// Capacity of the file mapping
	static std::shared_ptr<File>	s_nullptr;		// INVALID_HANDLE_VALUE file
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MAPPEDFILE_H_
