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
#include "BZip2StreamReader.h"

#pragma warning(push, 4)				

// COMPRESSION_METHOD
//
// Used when generating decompression exceptions
const tchar_t COMPRESSION_METHOD[] = _T("bzip2");

//-----------------------------------------------------------------------------
// BZip2StreamReader Constructor
//
// Arguments:
//
//	base		- Pointer to the start of the GZIP stream
//	length		- Length of the input stream, in bytes

BZip2StreamReader::BZip2StreamReader(const void* base, size_t length)
{
	if(!base) throw Exception(E_POINTER);
	if(length == 0) throw Exception(E_INVALIDARG);

#ifdef _WIN64
	if(length > UINT32_MAX) throw Exception(E_INVALIDARG);
#endif

	// Initialize the bzlib stream structure
	memset(&m_stream, 0, sizeof(bz_stream));
	m_stream.avail_in = static_cast<unsigned int>(length);
	m_stream.next_in  = reinterpret_cast<char*>(const_cast<void*>(base));;

	int result = BZ2_bzDecompressInit(&m_stream, 0, 0);
	if(result != BZ_OK) throw Exception(E_DECOMPRESS_INIT, COMPRESSION_METHOD);
}

//-----------------------------------------------------------------------------
// BZip2StreamReader Destructor

BZip2StreamReader::~BZip2StreamReader()
{
	BZ2_bzDecompressEnd(&m_stream);
}

//-----------------------------------------------------------------------------
// BZip2StreamReader::Read (StreamReader)
//
// Reads the specified number of bytes from the input stream into the output buffer
//
// Arguments:
//
//	buffer			- Output buffer
//	length			- Length of the output buffer, in bytes

size_t BZip2StreamReader::Read(void* buffer, size_t length)
{
	bool freemem = false;							// Flag to free buffer
	uint32_t out = m_stream.total_out_lo32;			// Save the current total

#ifdef _WIN64
	if(length > UINT32_MAX) throw Exception(E_INVALIDARG);
#endif

	if((length == 0) || (m_finished)) return 0;		// Nothing to do

	// The caller can specify NULL if the output data is irrelevant, but zlib
	// expects to be able to write the decompressed data somewhere ...
	if(!buffer) {

		buffer = malloc(length);
		if(!buffer) throw Exception(E_OUTOFMEMORY);
		freemem = true;
	}

	// Set the output buffer pointer and length for zlib
	m_stream.next_out = reinterpret_cast<char*>(buffer);
	m_stream.avail_out = static_cast<uint32_t>(length);

	// Inflate up to the requested number of bytes from the compressed stream
	int result = BZ2_bzDecompress(&m_stream);
	if(freemem) free(buffer);

	if((result != BZ_OK) && (result != BZ_STREAM_END))
		throw Exception(E_DECOMPRESS_CORRUPT, COMPRESSION_METHOD);

	// Prevent reading from beyond the end of the stream
	if(result == BZ_STREAM_END) m_finished = true;

	out = (m_stream.total_out_lo32 - out);		// Update output count
	m_position += out;							// Update stream position
	
	return out;
}

//-----------------------------------------------------------------------------
// BZip2StreamReader::Seek (StreamReader)
//
// Advances the stream to the specified position
//
// Arguments:
//
//	position		- Position to advance the input stream to

void BZip2StreamReader::Seek(size_t position)
{
#ifdef _WIN64
	if(position > UINT32_MAX) throw Exception(E_INVALIDARG);
#endif

	if(position < m_position) throw Exception(E_INVALIDARG);
	
	// Use Read() to decompress and advance the stream
	Read(NULL, position - m_position);
	if(m_position != position) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
