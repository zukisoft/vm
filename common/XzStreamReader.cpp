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
#include "XzStreamReader.h"				// Include XzStreamReader declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

// COMPRESSION_METHOD
//
// Used when generating decompression exceptions
const tchar_t COMPRESSION_METHOD[] = _T("xz");

//-----------------------------------------------------------------------------
// XzStreamReader Constructor
//
// Arguments:
//
//	base		- Pointer to the start of the GZIP stream
//	length		- Length of the input stream, in bytes

XzStreamReader::XzStreamReader(const void* base, size_t length)
{
	if(!base) throw Exception(E_POINTER);
	if(length == 0) throw Exception(E_INVALIDARG);

#ifdef _WIN64
	if(length > UINT32_MAX) throw Exception(E_INVALIDARG);
#endif

	// Maintain the original information for Reset() capability
	m_base = reinterpret_cast<uint8_t*>(const_cast<void*>(base));
	m_length = static_cast<uint32_t>(length);
	m_position = 0;
	m_finished = false;

	// Initialize the XZ CRC32 checksum generator
	xz_crc32_init();

	// Initialize the XZ buffer structure
	memset(&m_buffer, 0, sizeof(xz_buf));
	m_buffer.in = m_base;
	m_buffer.in_pos = 0;
	m_buffer.in_size = length;

	// Initialize the XZ decoder
	m_decoder = xz_dec_init(XZ_DYNALLOC, 1 << 26);
	if(!m_decoder) throw Exception(E_OUTOFMEMORY);
}

//-----------------------------------------------------------------------------
// XzStreamReader Destructor

XzStreamReader::~XzStreamReader()
{
	xz_dec_end(m_decoder);
}

//-----------------------------------------------------------------------------
// XzStreamReader::Read (StreamReader)
//
// Reads the specified number of bytes from the input stream into the output buffer
//
// Arguments:
//
//	buffer			- Output buffer
//	length			- Length of the output buffer, in bytes

uint32_t XzStreamReader::Read(void* buffer, uint32_t length)
{
	bool freemem = false;							// Flag to free buffer

	if((length == 0) || (m_finished)) return 0;		// Nothing to do

	// The caller can specify NULL if the output data is irrelevant, but xz
	// expects to be able to write the decompressed data somewhere ...
	if(!buffer) {

		buffer = malloc(length);
		if(!buffer) throw Exception(E_OUTOFMEMORY);
		freemem = true;
	}

	// Set the output buffer pointer and length for xz
	m_buffer.out = reinterpret_cast<uint8_t*>(buffer);
	m_buffer.out_pos = 0;
	m_buffer.out_size = length;

	// Decompress up to the requested number of bytes from the compressed stream
	xz_ret result = xz_dec_run(m_decoder, &m_buffer);
	if(freemem) free(buffer);

	// Check the result from xz_dec_run for memory errors
	if((result == XZ_MEM_ERROR) || (result == XZ_MEMLIMIT_ERROR)) 
		throw Exception(E_OUTOFMEMORY);

	// Assume any other bad thing that happened was due to a corrupt/unsupported input file
	if((result != XZ_OK) && (result != XZ_STREAM_END)) 
		throw Exception(E_DECOMPRESS_CORRUPT, COMPRESSION_METHOD);

	// XZ will raise an error on an attempt to read beyond the end
	if(result == XZ_STREAM_END) m_finished = true;

	m_position += static_cast<uint32_t>(m_buffer.out_pos);
	return static_cast<uint32_t>(m_buffer.out_pos);
}

//-----------------------------------------------------------------------------
// XzStreamReader::Reset (StreamReader)
//
// Resets the stream back to the beginning
//
// Arguments:
//
//	NONE

void XzStreamReader::Reset(void)
{
	// Reset the XZ decoder state
	xz_dec_reset(m_decoder);

	// Reset the XZ buffer input
	m_buffer.in = m_base;
	m_buffer.in_pos = 0;
	m_buffer.in_size = m_length;

	m_position = 0;
	m_finished = false;
}

//-----------------------------------------------------------------------------
// XzStreamReader::Seek (StreamReader)
//
// Advances the stream to the specified position
//
// Arguments:
//
//	position		- Position to advance the input stream to

void XzStreamReader::Seek(uint32_t position)
{
	if(position < m_position) throw Exception(E_INVALIDARG);
	
	// Use Read() to decompress and advance the stream
	Read(NULL, position - m_position);
	if(m_position != position) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
