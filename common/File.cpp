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

#include "stdafx.h"
#include "File.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// File Constructor (private)
//
// Arguments:
//
//	path			- Path to the file
//	access			- File access mask
//	share			- File sharing flags
//	disposition		- File creation disposition
//	flags			- File flags and attributes

File::File(const tchar_t* path, uint32_t access, uint32_t share, uint32_t disposition, uint32_t flags)
{
	if(!path || !(*path)) throw Exception(E_INVALIDARG);

	m_handle = CreateFile(path, access, share, NULL, disposition, flags, NULL);
	if(m_handle == INVALID_HANDLE_VALUE) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// File Destructor

File::~File()
{
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
// File::getSize
//
// Gets the size of the file

size_t File::getSize(void) const
{
	LARGE_INTEGER			size;				// Size of the file

	// Attempt to get the size of the file as a LARGE_INTEGER
	if(!GetFileSizeEx(m_handle, &size)) throw Win32Exception();

#ifdef _M_X64
	return static_cast<size_t>(size.QuadPart);
#else
	// Can only return up to INT32_MAX with a 32-bit size_t
	return (size.HighPart) ? INT32_MAX : static_cast<size_t>(size.LowPart);
#endif
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
