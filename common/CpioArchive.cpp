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
#include "CpioArchive.h"				// Include CpioArchive declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// AlignUp
//
// Aligns a numeric value up to a specific boundary
//
// Arguments:
//
//	value		- value to be aligned
//	alignment	- Alignment boundary

static uint32_t AlignUp(uint32_t value, size_t alignment)
{
	if(value == 0) return 0;
	else return value + ((alignment - (value % alignment)) % alignment);
}

//-----------------------------------------------------------------------------
// ConvertHexString
//
// Converts an ANSI hexadecimal string into a numeric value
//
// Arguments:
//
//	str		- Pointer to the hex string to convert
//	len		- Length of the hex string, not including any NULL terminator

static uint32_t ConvertHexString(const char* str, size_t len)
{
	uint32_t accumulator = 0;			// Value accumulator

	// Process until the end of the string or length reaches zero
	while((str != nullptr) && (*str != '\0') && (len-- > 0)) {

		char ch = *str++;			// Cast out for clarity and increment
		int delta;					// ASCII delta to get value

		if(!isxdigit(static_cast<int>(ch))) break;

		// Determine what the delta is for this character code
		if((ch >= '0') && (ch <= '9')) delta = 48;
		else if((ch >= 'A') && (ch <= 'F')) delta = 55;
		else if((ch >= 'a') && (ch <= 'f')) delta = 87;
		else return 0;

		accumulator = (accumulator << 4) + (ch - delta);
	}

	return accumulator;
}

//-----------------------------------------------------------------------------
// CpioFile Constructor
//
// Arguments:
//
//	basestream	- Reference to the base stream object, positioned at file data
//	header		- Reference to the CPIO file header
//	path		- File path extracted from the data stream

CpioFile::CpioFile(const cpio_header_t& header, const char_t* path, StreamReader& data)	
	: m_path(path), m_data(data)
{
	m_inode		= ConvertHexString(header.c_ino, 8);
	m_mode		= ConvertHexString(header.c_mode, 8);
	m_uid		= ConvertHexString(header.c_uid, 8);
	m_gid		= ConvertHexString(header.c_gid, 8);
	m_numlinks	= ConvertHexString(header.c_nlink, 8);
	m_mtime		= ConvertHexString(header.c_mtime, 8);
	m_devmajor	= ConvertHexString(header.c_maj, 8);
	m_devminor	= ConvertHexString(header.c_min, 8);
}

//-----------------------------------------------------------------------------
// CpioArchive::EnumerateFiles
//
// Enumerates over all of the files/objects contained in a CPIO archive
//
// Arguments:
//
//	reader		- StreamReader instance set to the beginning of the archive
//	func		- Function to process each entry in the archive

void CpioArchive::EnumerateFiles(std::unique_ptr<StreamReader>& reader, std::function<void(const CpioFile&&)> func)
{
	cpio_header_t			header;				// Current file header

	// Process each file embedded in the CPIO archive input stream
	while(reader->Read(&header, sizeof(cpio_header_t)) == sizeof(cpio_header_t)) {

		// CPIO header magic number is "070701" or "070702" if a checksum is present.
		// (I'm not bothering to test the checksum; can't be used to verify the file data)
		if(strncmp(header.c_magic, "07070", 5) != 0) return;
		if((header.c_magic[5] != '1') && (header.c_magic[5] != '2')) return;

		// Read the entry path string
		char_t path[_MAX_PATH];
		if(reader->Read(path, ConvertHexString(header.c_namesize, 8)) == 0) path[0] = '\0';
		
		// A path of "TRAILER!!!" indicates there are no more entries to process
		if(strcmp(path, "TRAILER!!!") == 0) return;

		// DWORD alignment for the file data in the archive
		reader->Seek(AlignUp(reader->Position, 4));

		// Create a FileStream around the current base stream position
		uint32_t datalength = ConvertHexString(header.c_filesize, 8);
		FileStream filestream(reader, datalength);

		// Invoke the caller-supplied function with a new CpioFile object
		func(CpioFile(header, path, filestream));

		// In the event the entire file stream was not read, seek beyond it and
		// apply the DWORD alignment to get to the next entry header
		reader->Seek(AlignUp(reader->Position + (datalength - filestream.Position), 4));
	}
}

//-----------------------------------------------------------------------------
// CpioArchive::FileStream::Read
//
// Reads data from the CPIO file stream
//
// Arguments:
//
//	buffer		- Destination buffer (can be NULL)
//	length		- Number of bytes to be read from the file stream

uint32_t CpioArchive::FileStream::Read(void* buffer, uint32_t length)
{
	// Check for null read and end-of-stream
	if((length == 0) || (m_position >= m_length)) return 0;

	// Do not read beyond the end of the length specified in the constructor
	if(m_position + length > m_length) length = (m_length - m_position);

	// Read the data from the base stream
	uint32_t out = m_basestream->Read(buffer, length);
	m_position += out;

	return out;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)