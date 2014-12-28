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

#include "stdafx.h"	
#include "MappedFile.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// Static Initializers

std::unique_ptr<File> MappedFile::s_nullptr(nullptr);

//-----------------------------------------------------------------------------
// MappedFile Constructor (private)
//
// Arguments:
//
//	file		- File to create mapping against or nullptr
//	protect		- Mapping protection flags
//	capacity	- Capacity of the file mapping to create
//	name		- Mapping name

MappedFile::MappedFile(const std::unique_ptr<File>& file, uint32_t protect, size_t capacity, const tchar_t* name)
{
	ULARGE_INTEGER		ulcapacity;			// Capacity as a ULARGE_INTEGER

	// A pagefile-backed mapping requires a capacity be specified
	if((file == nullptr) && (capacity == 0)) throw Exception(E_INVALIDARG);

	// Process the capacity as a ULARGE_INTEGER to more easily handle varying size_t
	ulcapacity.QuadPart = capacity;

	// Attempt to create the file mapping with the provided parameters
	m_handle = CreateFileMapping((file == nullptr) ? INVALID_HANDLE_VALUE : file->Handle, NULL, protect,
		ulcapacity.HighPart, ulcapacity.LowPart, name);
	if(!m_handle) throw Win32Exception();

	// If a file-based mapping was created with a zero capacity, use the file size
	if((file != nullptr) && (capacity == 0)) capacity = file->Size;
	
	m_capacity = capacity;					// Store requested capacity
}

//-----------------------------------------------------------------------------
// MappedFile Destructor

MappedFile::~MappedFile()
{
	if(m_handle) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
