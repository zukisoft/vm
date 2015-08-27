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
#include "BufferStreamReader.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// BufferStreamReader Constructor
//
// Arguments:
//
//	base		- Pointer to the start of the GZIP stream
//	length		- Length of the input stream, in bytes

BufferStreamReader::BufferStreamReader(const void* base, size_t length)
{
	if(!base) throw Exception(E_POINTER);
	if(length == 0) throw Exception(E_INVALIDARG);

#ifdef _WIN64
	if(length > UINT32_MAX) throw Exception(E_INVALIDARG);
#endif

	m_base = base;
	m_length = length;
	m_offset = 0;
}

//-----------------------------------------------------------------------------
// BufferStreamReader::Read (StreamReader)
//
// Reads the specified number of bytes from the input stream into the output buffer
//
// Arguments:
//
//	buffer			- Output buffer
//	length			- Length of the output buffer, in bytes

size_t BufferStreamReader::Read(void* buffer, size_t length)
{
	if(length == 0) return 0;				// Nothing to do

#ifdef _WIN64
	if(length > UINT32_MAX) throw Exception(E_INVALIDARG);
#endif

	// Calulate the number of output bytes that can be returned
	size_t out = std::min(length, m_length - m_offset);

	// Copy the memory from the source buffer into the output buffer.  The caller
	// can provide NULL if they just want to skip over some bytes ...
	if((buffer) && (out > 0)) memcpy(buffer, reinterpret_cast<const uint8_t*>(m_base) + m_offset, out);

	m_offset += out;				// Advance current offset
	return out;						// Return number of bytes copied
}

//-----------------------------------------------------------------------------
// BufferStreamReader::Seek (StreamReader)
//
// Advances the stream to the specified position
//
// Arguments:
//
//	position		- Position to advance the input stream to

void BufferStreamReader::Seek(size_t position)
{
#ifdef _WIN64
	if(position > UINT32_MAX) throw Exception(E_INVALIDARG);
#endif

	// For consistency with the compressed streams, this is a forward-only operation
	if((position < m_offset) || (position >= m_length)) throw Exception(E_INVALIDARG);
	m_offset = position;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
