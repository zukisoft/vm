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

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// MappedFile
//
// Creates a memory-mapped file

class MappedFile
{
public:

	// Constructors / Destructor
	//
	MappedFile(HANDLE file, DWORD protect, size_t length);
	~MappedFile();

	//-------------------------------------------------------------------------
	// Properties

	// Handle
	//
	// Gets the underlying handle for the mapped file
	__declspec(property(get=getHandle)) HANDLE Handle;
	void* getHandle(void) const { return m_handle; }

	// Length
	//
	// Gets the length of the memory mapped file
	__declspec(property(get=getLength)) size_t Length;
	size_t getLength(void) const { return m_length; }

private:

	MappedFile(const MappedFile&);
	MappedFile& operator=(const MappedFile&);

	//-------------------------------------------------------------------------
	// Member Variables

	HANDLE				m_handle;			// File mapping handle
	size_t				m_length;			// Length of the file mapping
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MAPPEDFILE_H_
