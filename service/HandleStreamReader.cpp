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
#include "HandleStreamReader.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// HandleStreamReader::Read
//
// Reads data from a FileSystem::Handle instance
//
// Arguments:
//
//	buffer		- Destination buffer
//	length		- Number of bytes to be read from the file stream

size_t HandleStreamReader::Read(void* buffer, size_t length)
{
	// Ask the Handle instance to read the data into the destination
	size_t read = m_handle->Read(buffer, length);

	m_position += read;				// Update stream position
	return read;					// Return bytes read
}

//-----------------------------------------------------------------------------
// HandleStreamReader::Seek
//
// Seeks the stream to the specified position
//
// Arguments:
//
//	position		- Position to set the stream pointer to

void HandleStreamReader::Seek(size_t position)
{
	// Handles use uapi::loff_t, which is a signed 64-bit value
	if(position > INT64_MAX) throw Exception(E_INVALIDARG);

	// StreamReaders are supposed to be forward-only; even though the Handle
	// can technically support this, follow the interface contract
	// TODO: Removed this to fix something temporarily; StreamReader is not the
	// best choice for dealing with ELF images
	///if(position < m_position) throw Exception(E_INVALIDARG);

	// Attempt to set the file pointer to the specified position
	uapi::loff_t offset = static_cast<uapi::loff_t>(position);
	if(m_handle->Seek(offset, LINUX_SEEK_SET) != offset) throw Exception(E_INVALIDARG);

	m_position = position;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
