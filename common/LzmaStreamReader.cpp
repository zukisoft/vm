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
#include "LzmaStreamReader.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

// COMPRESSION_METHOD
//
// Used when generating decompression exceptions
const tchar_t COMPRESSION_METHOD[] = _T("lzma");

// LzmaAlloc
//
// LZMA memory allocator
static void* LzmaAlloc(void*, size_t size) { return new uint8_t[size]; }

// LzmaFree
//
// LZMA memory deallocator
static void LzmaFree(void*, void* address) { delete[] reinterpret_cast<uint8_t*>(address); }

// g_szalloc
//
// ISzAlloc implementation 
static ISzAlloc g_szalloc = { LzmaAlloc, LzmaFree };

//-----------------------------------------------------------------------------
// LzmaStreamReader Constructor
//
// Arguments:
//
//	base		- Pointer to the start of the LZMA stream
//	length		- Length of the input stream, in bytes

LzmaStreamReader::LzmaStreamReader(const void* base, size_t length) : m_baseptr(uintptr_t(base)), m_length(length)
{
	if(!base) throw Exception(E_POINTER);
	if(length == 0) throw Exception(E_INVALIDARG);

#ifdef _WIN64
	if(length > UINT32_MAX) throw Exception(E_INVALIDARG);
#endif

	// Initialize the LZMA state structure
	LzmaDec_Construct(&m_state);

	// Input buffer must be large enough to at least contain the header and stream length
	if(length <= (LZMA_PROPS_SIZE + sizeof(uint64_t))) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);

	// Get the length of the output data, if known, and set the input pointer after the header
	m_streamlen = *reinterpret_cast<uint64_t*>(m_baseptr + LZMA_PROPS_SIZE);
	m_inputptr = m_baseptr + LZMA_PROPS_SIZE + sizeof(uint64_t);

#ifdef _WIN64
	// If the stream length is known and is more than can fit in UINT32, that's a problem for Read()
	if((m_streamlen != UINT64_MAX) && (m_streamlen > UINT32_MAX)) throw Exception(E_DECOMPRESS_TOOBIG, COMPRESSION_METHOD);
#endif

	// Initialize the LZMA allocator, check for the possible failure codes and throw accordingly
	SRes result = LzmaDec_Allocate(&m_state, reinterpret_cast<uint8_t*>(m_baseptr), LZMA_PROPS_SIZE, &g_szalloc);
	if(result == SZ_ERROR_MEM) throw Exception(E_OUTOFMEMORY);
	else if(result == SZ_ERROR_UNSUPPORTED) throw Exception(E_DECOMPRESS_CORRUPT, COMPRESSION_METHOD);

	LzmaDec_Init(&m_state);				// Initialize the LZMA stream decoder
}

//-----------------------------------------------------------------------------
// LzmaStreamReader Destructor

LzmaStreamReader::~LzmaStreamReader()
{
	LzmaDec_Free(&m_state, &g_szalloc);
}

//-----------------------------------------------------------------------------
// LzmaStreamReader::Read (StreamReader)
//
// Reads the specified number of bytes from the input stream into the output buffer
//
// Arguments:
//
//	buffer			- Output buffer
//	length			- Length of the output buffer, in bytes

uint32_t LzmaStreamReader::Read(void* buffer, uint32_t length)
{
	ELzmaStatus			status;						// Decompression status
	bool				freemem = false;			// Flag to free output buffer

	if((length == 0) || (m_finished)) return 0;		// Nothing to do

	// The caller can specify NULL if the output data is irrelevant, but lzma
	// expects to be able to write the decompressed data somewhere ...
	if(!buffer) {

		buffer = malloc(length);
		if(!buffer) throw Exception(E_OUTOFMEMORY);
		freemem = true;
	}

	// The number of input bytes is the length of the source buffer less position
	size_t inputlen = m_length - (m_inputptr - m_baseptr);

	// LZMA_FINISH_END can be specified when it's known that this is the last block
	ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
	if(m_position + length >= m_streamlen) finishMode = LZMA_FINISH_END;

	// Attempt to decode the next block of compressed data into the output buffer
	SRes result = LzmaDec_DecodeToBuf(&m_state, reinterpret_cast<uint8_t*>(buffer), &length,
		reinterpret_cast<uint8_t*>(m_inputptr), &inputlen, finishMode, &status);
	
	// Always release a locally allocated output buffer and check for SZ_ERROR_DATA
	if(freemem) free(buffer);
	if(result == SZ_ERROR_DATA) throw Exception(E_DECOMPRESS_CORRUPT, COMPRESSION_METHOD);

	switch(status) {

		// LZMA_STATUS_NOT_FINISHED: There is more data in the stream
		//
		case LZMA_STATUS_NOT_FINISHED: break;

		// LZMA_STATUS_FINISHED_WITH_MARK / LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK: End of stream
		//
		case LZMA_STATUS_FINISHED_WITH_MARK:
		case LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK: m_finished = true; break;

		// LZMA_STATUS_NEEDS_MORE_INPUT: The input data has been truncated
		//
		case LZMA_STATUS_NEEDS_MORE_INPUT: throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);

		// Everything else: General bad thing; assume the file is corrupted
		//
		default: throw Exception(E_DECOMPRESS_CORRUPT, COMPRESSION_METHOD);
	}

	m_inputptr += inputlen;					// Advance the input buffer pointer
	m_position += length;					// Advance the output stream position

	return length;
}

//-----------------------------------------------------------------------------
// LzmaStreamReader::Seek (StreamReader)
//
// Advances the stream to the specified position
//
// Arguments:
//
//	position		- Position to advance the input stream to

void LzmaStreamReader::Seek(uint32_t position)
{
	if(position < m_position) throw Exception(E_INVALIDARG);
	
	// Use Read() to decompress and advance the stream to the new position
	Read(NULL, position - m_position);
	if(m_position != position) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
