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
#include "Lz4StreamReader.h"
#pragma warning(push, 4)				

// COMPRESSION_METHOD
//
// Used when generating decompression exceptions
const tchar_t COMPRESSION_METHOD[] = _T("lz4");

//-----------------------------------------------------------------------------
// LZ4 DECLARATIONS
//-----------------------------------------------------------------------------

#define MAGICNUMBER_SIZE	4
#define LEGACY_MAGICNUMBER	0x184C2102
#define LEGACY_BLOCKSIZE	(8 << 20)

//-----------------------------------------------------------------------------
// ReadLE32
//
// Reads a little endian uint32_t from the input stream
//
// Arguments:
//
//	base		- Current base pointer in the stream
//	length		- Remaining length of the stream [in/out]
//	value		- Value read from the stream [out]

static intptr_t ReadLE32(intptr_t base, size_t* length, uint32_t* value)
{
	if((!length) | (!value)) throw Exception(E_POINTER);
	if(*length < sizeof(uint32_t)) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);

	*value = *reinterpret_cast<uint32_t*>(base);

	*length -= sizeof(uint32_t);
	return base + sizeof(uint32_t);
}

//-----------------------------------------------------------------------------
// Lz4StreamReader Constructor
//
// Arguments:
//
//	base		- Pointer to the start of the GZIP stream
//	length		- Length of the input stream, in bytes

Lz4StreamReader::Lz4StreamReader(const void* base, size_t length)
{
	if(!base) throw Exception(E_POINTER);
	if(length == 0) throw Exception(E_INVALIDARG);

#ifdef _WIN64
	if(length > UINT32_MAX) throw Exception(E_INVALIDARG);
#endif

	intptr_t baseptr = intptr_t(base);

	// Read and validate the magic number from the start of the stream
	uint32_t magic;
	baseptr = ReadLE32(baseptr, &length, &magic);
	if(magic != LEGACY_MAGICNUMBER) throw Exception(E_DECOMPRESS_BADMAGIC, COMPRESSION_METHOD);

	// Allocate the decompression buffer
	m_block = new uint8_t[LEGACY_BLOCKSIZE];
	if(!m_block) throw Exception(E_OUTOFMEMORY);

	// Initialize the decompression block member variables
	m_blockcurrent = m_block;
	m_blockremain = 0;

	// Initialize the LZ4 input stream member variables
	m_lz4pos = baseptr;
	m_lz4remain = length;
}

//-----------------------------------------------------------------------------
// Lz4StreamReader Destructor

Lz4StreamReader::~Lz4StreamReader()
{
	if(m_block) delete[] m_block;
}

//-----------------------------------------------------------------------------
// Lz4StreamReader::ReadNextBlock (private)
//
// Reads the next block of data from the input stream
//
// Arguments:
//
//	NONE

uint32_t Lz4StreamReader::ReadNextBlock(void)
{
	// No more data
	if(m_lz4remain == 0) return 0;

	// Get the amount of compressed data in the next block
	uint32_t compressed;
	m_lz4pos = ReadLE32(m_lz4pos, &m_lz4remain, &compressed);

	// Read the next block of data from the compression stream
	int uncompressed = LZ4_decompress_safe(reinterpret_cast<const char*>(m_lz4pos), reinterpret_cast<char*>(m_block),
		compressed, LEGACY_BLOCKSIZE);
	if(uncompressed < 0) throw Exception(E_DECOMPRESS_CORRUPT, COMPRESSION_METHOD);

	// Advance the stream pointers to the next block of data
	m_lz4pos += compressed;
	m_lz4remain -= compressed;

	// Detect the end of the input stream by have less than a full block of data returned
	if(uncompressed < LEGACY_BLOCKSIZE) {

		m_lz4pos += m_lz4remain;
		m_lz4remain = 0;
	}

	// Reset the block current pointer and the number of remaining bytes
	m_blockcurrent = m_block;
	m_blockremain = static_cast<uint32_t>(uncompressed);

	return uncompressed;
}

//-----------------------------------------------------------------------------
// Lz4StreamReader::Read (StreamReader)
//
// Reads the specified number of bytes from the input stream into the output buffer
//
// Arguments:
//
//	buffer			- Output buffer; can be NULL
//	length			- Length of the output buffer, in bytes

size_t Lz4StreamReader::Read(void* buffer, size_t length)
{
	uint32_t		out = 0;				// Bytes returned to caller

#ifdef _WIN64
	if(length > UINT32_MAX) throw Exception(E_INVALIDARG);
#endif

	if(length == 0) return 0;				// Nothing to do

	// Read uncompressed data into the output buffer until either
	// the specified amount has been read or the stream ends
	while(length > 0) {

		// Take the smaller of what we have and what we still need
		uint32_t next = std::min(m_blockremain, static_cast<uint32_t>(length));
		if(next) {
			
			// The buffer pointer can be NULL to just skip over data
			if(buffer) memcpy(buffer, m_blockcurrent, next);

			// Move the block data pointer
			m_blockcurrent += next;
			m_blockremain -= next;
		
			// Move the output buffer pointer
			if(buffer) buffer = reinterpret_cast<uint8_t*>(buffer) + next;
			length -= next;

			out += next;			// Wrote this many output bytes
		}

		// If there is no more block data, decompress the next block; if
		// there is no more data in the stream, we're done
		if((m_blockremain == 0) && (ReadNextBlock() == 0)) break;
	}

	m_position += out;			// Increment the current stream position
	return out;					// Return number of bytes written
}

//-----------------------------------------------------------------------------
// Lz4StreamReader::Seek (StreamReader)
//
// Advances the stream to the specified position
//
// Arguments:
//
//	position		- Position to advance the input stream to

void Lz4StreamReader::Seek(size_t position)
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
