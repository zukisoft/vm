//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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
#include "LzopStreamReader.h"

#pragma warning(push, 4)				

// COMPRESSION_METHOD
//
// Used when generating decompression exceptions
const tchar_t COMPRESSION_METHOD[] = _T("lzop");

//-----------------------------------------------------------------------------
// LZOP DECLARATIONS
//-----------------------------------------------------------------------------

// lzopMagic
//
// LZOP file header magic number
static uint8_t lzopMagic[] = { 0x89, 'L', 'Z', 'O', 0x00, 0x0D, 0x0A, 0x1A, 0x0A };

// MAX_BLOCK_SIZE
//
// Maximum allowed input block size
#define MAX_BLOCK_SIZE	(64 * 1024L * 1024L)

// ADLER32_INIT_VALUE
//
// Seed value for lzo_adler32() function
#define ADLER32_INIT_VALUE	1

// LZOP header flags
//
#define F_ADLER32_D     0x00000001L
#define F_ADLER32_C     0x00000002L
#define F_STDIN         0x00000004L
#define F_STDOUT        0x00000008L
#define F_NAME_DEFAULT  0x00000010L
#define F_DOSISH        0x00000020L
#define F_H_EXTRA_FIELD 0x00000040L
#define F_H_GMTDIFF     0x00000080L
#define F_CRC32_D       0x00000100L
#define F_CRC32_C       0x00000200L
#define F_MULTIPART     0x00000400L
#define F_H_FILTER      0x00000800L
#define F_H_CRC32       0x00001000L
#define F_H_PATH        0x00002000L
#define F_MASK          0x00003FFFL

// header_t
//
// LZOP file header structure
typedef struct {
    
	unsigned short	version;
	unsigned short	lib_version;
	unsigned short	version_needed_to_extract;
	unsigned char	method;
	unsigned char	level;
	lzo_uint32		flags;
	lzo_uint32		filter;
	lzo_uint32		mode;
	lzo_uint32		mtime_low;
	lzo_uint32		mtime_high;
	lzo_uint32		header_checksum;

	lzo_uint32		extra_field_len;
	lzo_uint32		extra_field_checksum;

} header_t;

//-----------------------------------------------------------------------------
// ReadBE8
//
// Reads a big endian uint8_t from the input stream
//
// Arguments:
//
//	base		- Current base pointer in the stream
//	length		- Remaining length of the stream [in/out]
//	value		- Value read from the stream [out]

static intptr_t ReadBE8(intptr_t base, size_t* length, uint8_t* value)
{
	if((!length) | (!value)) throw Exception(E_POINTER);
	if(*length < sizeof(uint8_t)) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);

	// Of course, there is no difference in endianness of a single byte,
	// this function is for consistency with ReadBE16() and ReadBE32()
	*value = *reinterpret_cast<uint8_t*>(base);

	*length -= sizeof(uint8_t);
	return base + sizeof(uint8_t);
}

//-----------------------------------------------------------------------------
// ReadBE16
//
// Reads a big endian uint16_t from the input stream
//
// Arguments:
//
//	base		- Current base pointer in the stream
//	length		- Remaining length of the stream [in/out]
//	value		- Value read from the stream [out]

static intptr_t ReadBE16(intptr_t base, size_t* length, uint16_t* value)
{
	if((!length) | (!value)) throw Exception(E_POINTER);
	if(*length < sizeof(uint16_t)) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);

	*value = _byteswap_ushort(*reinterpret_cast<uint16_t*>(base));

	*length -= sizeof(uint16_t);
	return base + sizeof(uint16_t);
}

//-----------------------------------------------------------------------------
// ReadBE32
//
// Reads a big endian uint32_t from the input stream
//
// Arguments:
//
//	base		- Current base pointer in the stream
//	length		- Remaining length of the stream [in/out]
//	value		- Value read from the stream [out]

static intptr_t ReadBE32(intptr_t base, size_t* length, uint32_t* value)
{
	if((!length) | (!value)) throw Exception(E_POINTER);
	if(*length < sizeof(uint32_t)) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);

	*value = _byteswap_ulong(*reinterpret_cast<uint32_t*>(base));

	*length -= sizeof(uint32_t);
	return base + sizeof(uint32_t);
}

//-----------------------------------------------------------------------------
// ReadHeader
//
// Reads the LZOP header from the head of the input stream
//
// Arguments:
//
//	base		- Current base pointer in the stream
//	length		- Remaining length of the stream [in/out]
//	header		- LZOP header structure to receive data [out]

static intptr_t ReadHeader(intptr_t base, size_t *length, header_t* header)
{
	if((!length) || (!header)) throw Exception(E_POINTER);

	// Get the LZO API version from the minilzo library
	uint16_t version = static_cast<uint16_t>(lzo_version());

	memset(header, 0, sizeof(header_t));
	header->version_needed_to_extract = 0x0900;

	// version
	base = ReadBE16(base, length, &header->version);
	if(header->version < 0x0900) throw Exception(E_DECOMPRESS_BADHEADER, COMPRESSION_METHOD);

	// lib_version
	base = ReadBE16(base, length, &header->lib_version);

	// version_needed_to_extract
	if(header->version >= 0x0940) {
		
		base = ReadBE16(base, length, &header->version_needed_to_extract);
		if(header->version_needed_to_extract > version) throw Exception(E_DECOMPRESS_BADHEADER, COMPRESSION_METHOD);
		if(header->version_needed_to_extract < 0x0900) throw Exception(E_DECOMPRESS_BADHEADER, COMPRESSION_METHOD);
	}

	// method
	base = ReadBE8(base, length, &header->method);

	// level
	if(header->version >= 0x0940)
		base = ReadBE8(base, length, &header->level);

	// flags
	base = ReadBE32(base, length, &header->flags);

	// filter
	if(header->flags & F_H_FILTER) 
		base = ReadBE32(base, length, &header->filter);

	// mode
	base = ReadBE32(base, length, &header->mode);
	if(header->flags & F_STDIN) header->mode = 0;

	// mtime_low
	base = ReadBE32(base, length, &header->mtime_low);

	// mtime_high
	if(header->version >= 0x0940)
		base = ReadBE32(base, length, &header->mtime_high);

	// filename (skipped)
	uint8_t fnlen;
	base = ReadBE8(base, length, &fnlen);
	if(fnlen) { base += fnlen; *length -= fnlen; }

	// checksum
	base = ReadBE32(base, length, &header->header_checksum);
	// TODO: check this??

	// skip extra fields
	if(header->flags & F_H_EXTRA_FIELD) {

		// length of extra data
		uint32_t extralen;
		base = ReadBE32(base, length, &extralen);
		base += extralen;
		*length -= extralen;

		// extra fields checksum
		uint32_t extrachecksum;
		base = ReadBE32(base, length, &extrachecksum);
	}

	return base;			// Return adjusted base pointer
}

//-----------------------------------------------------------------------------
// ReadMagic
//
// Reads the magic number from the head of the input stream
//
// Arguments:
//
//	base		- Current base pointer in the stream
//	length		- Remaining length of the stream [in/out]

static intptr_t ReadMagic(intptr_t base, size_t* length)
{
	if(!length) return E_POINTER;
	if(*length < sizeof(lzopMagic)) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);

	// Verify the magic number
	if(memcmp(reinterpret_cast<const void*>(base), &lzopMagic[0], sizeof(lzopMagic)) != 0)
		throw Exception(E_DECOMPRESS_BADMAGIC, COMPRESSION_METHOD);

	// Reduce the available length by magic number size and return adjusted pointer
	*length -= sizeof(lzopMagic);
	return base + sizeof(lzopMagic);
}

//-----------------------------------------------------------------------------
// LzopStreamReader Constructor
//
// Arguments:
//
//	base		- Pointer to the start of the GZIP stream
//	length		- Length of the input stream, in bytes

LzopStreamReader::LzopStreamReader(const void* base, size_t length)
{
	header_t			header;				// LZOP header information

	if(!base) throw Exception(E_POINTER);
	if(length == 0) throw Exception(E_INVALIDARG);

	intptr_t baseptr = intptr_t(base);

	// Verify the magic number and read the LZOP header information
	baseptr = ReadMagic(baseptr, &length);
	baseptr = ReadHeader(baseptr, &length, &header);

	// Initialize the decompression block member variables
	m_block = m_blockcurrent = NULL;
	m_blocklen = 0;
	m_blockremain = 0;

	// Initialize the LZO input stream member variables
	m_lzopos = baseptr;
	m_lzoremain = length;
	m_lzoflags = header.flags;
}

//-----------------------------------------------------------------------------
// LzopStreamReader Destructor

LzopStreamReader::~LzopStreamReader()
{
	if(m_block) delete[] m_block;
}

//-----------------------------------------------------------------------------
// LzopStreamReader::ReadNextBlock (private)
//
// Reads the next block of data from the input stream
//
// Arguments:
//
//	NONE

uint32_t LzopStreamReader::ReadNextBlock(void)
{
	uint32_t			uncompressed;			// Length of uncompressed data
	uint32_t			compressed;				// Length of compressed data
	uint32_t			adler_checksum_d = 0;	// Decompressed data ADLER32
	uint32_t			crc32_checksum_d = 0;	// Decompressed data CRC32
	uint32_t			adler_checksum_c = 0;	// Compressed data ADLER32
	uint32_t			crc32_checksum_c = 0;	// Compressed data CRC32

	// No more data
	if(m_lzoremain == 0) return 0;

	// Get the amount of uncompressed data in the next block
	m_lzopos = ReadBE32(m_lzopos, &m_lzoremain, &uncompressed);
	if(uncompressed == 0) {

		// Force data pointer to the end of the provided length
		// to prevent any more data from being read
		m_lzopos += m_lzoremain;
		m_lzoremain = 0;
		return 0;
	}

	// Read the length of the compressed data
	m_lzopos = ReadBE32(m_lzopos, &m_lzoremain, &compressed);

	// Read checksums
	if(m_lzoflags & F_ADLER32_D) m_lzopos = ReadBE32(m_lzopos, &m_lzoremain, &adler_checksum_d);
	if(m_lzoflags & F_CRC32_D) m_lzopos = ReadBE32(m_lzopos, &m_lzoremain, &crc32_checksum_d);
	if(m_lzoflags & F_ADLER32_C) m_lzopos = ReadBE32(m_lzopos, &m_lzoremain, &adler_checksum_c);
	if(m_lzoflags & F_CRC32_C) m_lzopos = ReadBE32(m_lzopos, &m_lzoremain, &crc32_checksum_c);

	// Sanity checks
	if(uncompressed > MAX_BLOCK_SIZE) throw Exception(E_DECOMPRESS_CORRUPT, COMPRESSION_METHOD);
	if(compressed > m_lzoremain) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);

	// Reallocate the block buffer if the current one is not large enough
	if(uncompressed > m_blocklen) {

		if(m_block) { delete[] m_block; m_block = NULL; }
		m_block = new uint8_t[uncompressed];
		if(!m_block) throw Exception(E_OUTOFMEMORY);
		m_blocklen = uncompressed;
	}

	// Special Case: uncompressed == compressed -> copy the data
	if(uncompressed == compressed)
		memcpy(m_block, reinterpret_cast<const void*>(m_lzopos), uncompressed);

	// Not the same, decompress the next block of data into the block buffer
	else {

		lzo_uint out = uncompressed;
		lzo1x_decompress(reinterpret_cast<const lzo_bytep>(m_lzopos), static_cast<lzo_uint>(compressed), m_block, &out, LZO1X_MEM_DECOMPRESS);
		if(out != uncompressed) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);
	}

#ifdef _DEBUG
	// Validate the checksum of the decompressed block data (adler32 only - minilzo doesn't have crc32)
	if(m_lzoflags & F_ADLER32_D) {

		uint32_t adler = lzo_adler32(ADLER32_INIT_VALUE, m_block, uncompressed);
		if(adler != adler_checksum_d) throw Exception(E_DECOMPRESS_CORRUPT, COMPRESSION_METHOD);
	}
#endif

	// Move the LZO stream pointer to the next block of data
	m_lzopos += compressed;
	m_lzoremain -= compressed;

	// Reset the block current pointer and the number of remaining bytes
	m_blockcurrent = m_block;
	m_blockremain = uncompressed;

	return uncompressed;
}

//-----------------------------------------------------------------------------
// LzopStreamReader::Read (StreamReader)
//
// Reads the specified number of bytes from the input stream into the output buffer
//
// Arguments:
//
//	buffer			- Output buffer; can be NULL
//	length			- Length of the output buffer, in bytes

size_t LzopStreamReader::Read(void* buffer, size_t length)
{
	size_t			out = 0;				// Bytes returned to caller

	if(length == 0) return 0;				// Nothing to do

	// Read uncompressed data into the output buffer until either
	// the specified amount has been read or the stream ends
	while(length > 0) {

		// Take the smaller of what we have and what we still need
		size_t next = std::min(m_blockremain, length);
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
// LzopStreamReader::Seek (StreamReader)
//
// Advances the stream to the specified position
//
// Arguments:
//
//	position		- Position to advance the input stream to

void LzopStreamReader::Seek(size_t position)
{
	if(position < m_position) throw Exception(E_INVALIDARG);
	
	// Use Read() to decompress and advance the stream
	Read(NULL, position - m_position);
	if(m_position != position) throw Exception(E_DECOMPRESS_TRUNCATED, COMPRESSION_METHOD);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
