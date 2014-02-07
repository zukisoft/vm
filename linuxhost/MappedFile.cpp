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

#include "stdafx.h"						// Include project pre-compiled headers
#include "MappedFile.h"					// Include MappedFile declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// MappedFile Constructor
//
// Arguments:
//
//	handle		- Handle from CreateFile() or NULL
//	protect		- Mapping protection flags
//	length		- Length of the file mapping to create

MappedFile::MappedFile(HANDLE file, DWORD protect, size_t length)
{
	ULARGE_INTEGER		ullength;			// Length as a ULARGE_INTEGER

	if(length == 0) throw Exception(E_INVALIDARG);

	// Process the length as a ULARGE_INTEGER to more easily deal with varying size_t
	ullength.QuadPart = length;

	// Attempt to create the file mapping with the specified parameters
	m_handle = CreateFileMapping(file, NULL, protect, ullength.HighPart, ullength.LowPart, NULL);
	if(!m_handle) throw Win32Exception();

	m_length = length;						// Record the length for convenience
}

//-----------------------------------------------------------------------------
// MappedFileView Destructor

MappedFile::~MappedFile()
{
	if(m_handle) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
