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

#ifndef __FILE_H_
#define __FILE_H_
#pragma once

#include <memory>
#include "char_t.h"
#include "Exception.h"
#include "Win32Exception.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// File
//
// Wrapper around a windows file handle

class File
{
public:

	// Destructor
	//
	~File();

	//-------------------------------------------------------------------------
	// Overloaded Operators
	
	// HANDLE
	//
	operator HANDLE() const { return m_handle; }

	//-------------------------------------------------------------------------
	// Member Functions

	// OpenExisting
	//
	// Opens an existing file

	static std::unique_ptr<File> OpenExisting(const tchar_t* path)
		{ return std::unique_ptr<File>(new File(path, GENERIC_READ | GENERIC_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL)); }

	static std::unique_ptr<File> OpenExisting(const tchar_t* path, uint32_t access)
		{ return std::unique_ptr<File>(new File(path, access, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL)); }
	
	static std::unique_ptr<File> OpenExisting(const tchar_t* path, uint32_t access, uint32_t share)
		{ return std::unique_ptr<File>(new File(path, access, share, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL)); }

	static std::unique_ptr<File> OpenExisting(const tchar_t* path, uint32_t access, uint32_t share, uint32_t flags)
		{ return std::unique_ptr<File>(new File(path, access, share, OPEN_EXISTING, flags)); }

	//-------------------------------------------------------------------------
	// Properties

	// Handle
	//
	// Gets the underlying handle for the file
	__declspec(property(get=getHandle)) HANDLE Handle;
	HANDLE getHandle(void) const { return m_handle; }

	// Size
	//
	// Gets the size of the file
	__declspec(property(get=getSize)) size_t Size;
	size_t getSize(void) const;

private:

	File(const File&);
	File& operator=(const File&);

	// Instance Constructor
	//
	File(const tchar_t* path, uint32_t access, uint32_t share, uint32_t disposition, uint32_t flags);

	//-------------------------------------------------------------------------
	// Member Variables

	HANDLE				m_handle;			// Contained file handle
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILE_H_
