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
#include "GZipStreamReader.h"			// Include GZipStreamReader declarations

#include "Exception.h"					// Include Exception class declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// GZipStreamReader Constructor
//
// Arguments:
//
//	base		- Pointer to the start of the GZIP stream
//	length		- Length of the input stream, in bytes

GZipStreamReader::GZipStreamReader(const void* base, size_t length)
{
	if(!base) throw Exception(E_POINTER);
	if((length == 0) || (length > UINT32_MAX)) throw Exception(E_INVALIDARG);

	// Initialize the zlib stream structure
	m_stream.zalloc		= Z_NULL;
	m_stream.zfree		= Z_NULL;
	m_stream.opaque		= Z_NULL;
	m_stream.avail_in	= static_cast<uint32_t>(length);
	m_stream.next_in	= reinterpret_cast<uint8_t*>(const_cast<void*>(base));

	// inflateInit2() must be used when working with a GZIP stream
	int result = inflateInit2(&m_stream, 16 + MAX_WBITS);
	if(result != Z_OK) throw Exception(E_FAIL, _T("Unable to initialize GZIP inflation stream"));
}

//-----------------------------------------------------------------------------
// GZipStreamReader Destructor

GZipStreamReader::~GZipStreamReader()
{
	// Clean up and release any zlib resources
	inflateEnd(&m_stream);
}

//-----------------------------------------------------------------------------
// GZipStreamReader::Read (StreamReader)
//
// Reads the specified number of bytes from the input stream into the output buffer
//
// Arguments:
//
//	buffer			- Output buffer
//	length			- Length of the output buffer, in bytes

uint32_t GZipStreamReader::Read(void* buffer, uint32_t length)
{
	uint32_t out = m_stream.total_out;		// Save the current total

	// Set the output buffer pointer and length for zlib
	m_stream.next_out = reinterpret_cast<uint8_t*>(buffer);
	m_stream.avail_out = length;

	// Inflate up to the requested number of bytes from the compressed stream
	int result = inflate(&m_stream, Z_SYNC_FLUSH);
	if((result != Z_OK) || (result != Z_STREAM_END))
		throw Exception(E_FAIL, _T("Unable to inflate GZIP stream data"));

	// Return the number of bytes inflated into the output buffer
	return m_stream.total_out - out;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
