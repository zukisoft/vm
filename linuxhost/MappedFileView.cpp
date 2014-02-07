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
#include "MappedFileView.h"				// Include MappedFileView declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// MappedFileView Constructor
//
// Arguments:
//
//	mapping		- Shared MappedFile instance
//	access		- Access flags for MapViewOfFile()
//	offset		- Offset into the file mapping to begin the view
//	length		- Length of the view to create

MappedFileView::MappedFileView(std::shared_ptr<MappedFile> mapping, DWORD access, size_t offset, size_t length)
{
	ULARGE_INTEGER		uloffset;				// Offset as a ULARGE_INTEGER

	m_mapping = mapping;						// Store the shared_ptr object

	// Process the offset as a ULARGE_INTEGER to more easily deal with varying size_t
	uloffset.QuadPart = offset;

	// Attempt to map the specified region of the file into this process
	m_view = MapViewOfFile(m_mapping->Handle, access, uloffset.HighPart, uloffset.LowPart, length);
	if(!m_view) throw Win32Exception();

	// If the specified length was zero, the view encompasses the entire mapped file
	m_length = (length) ? length : mapping->Length;
}

//-----------------------------------------------------------------------------
// MappedFileView Destructor

MappedFileView::~MappedFileView()
{
	if(m_view) UnmapViewOfFile(m_view);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
