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
#include "BufferStreamReader.h"			// Include BufferStreamReader declarations

#include "Exception.h"					// Include Exception class declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

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
	m_length = static_cast<uint32_t>(length);
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

uint32_t BufferStreamReader::Read(void* buffer, uint32_t length)
{
	if(length == 0) return 0;				// Nothing to do

	// Calulate the number of output bytes that can be returned
	uint32_t out = min(length, m_length - m_offset);

	// Copy the memory from the source buffer into the output buffer.  The caller
	// can provide NULL if they just want to skip over some bytes ...
	if((buffer) && (out > 0)) memcpy(buffer, reinterpret_cast<const uint8_t*>(m_base) + m_offset, out);

	// Move the current offset and return the number of bytes copied
	m_offset += out;
	return static_cast<uint32_t>(out);
}

//-----------------------------------------------------------------------------
// BufferStreamReader::Reset (StreamReader)
//
// Resets the stream back to the beginning
//
// Arguments:
//
//	NONE

void BufferStreamReader::Reset(void)
{
	m_offset = 0;
}

//-----------------------------------------------------------------------------
// BufferStreamReader::Seek (StreamReader)
//
// Advances the stream to the specified position
//
// Arguments:
//
//	position		- Position to advance the input stream to

void BufferStreamReader::Seek(uint32_t position)
{
	// For consistency with the compressed streams, this is a forward-only operation
	if((position < m_offset) || (position >= m_length)) throw Exception(E_INVALIDARG);
	m_offset = position;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
