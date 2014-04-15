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

//-----------------------------------------------------------------------------
// EXTERNAL DEPENDENCY: ZLIB
//
// - Add the following files from external\zlib to the parent project:
//
// adler32.c
// crc32.c
// inffast.c
// inflate.c
// inftrees.c
// zconf.h
// zlib.h
// zutil.c
//
// - Disable precompiled headers for all the above .c files
// - Define NO_GZ_COMPRESS as a preprocessor definition for all the above .c files
// - Add external\zlib to the project Additional Include Directories
// - Add #include <zlib.h> to the project stdafx.h header file
//-----------------------------------------------------------------------------

#ifndef __GZIPSTREAMREADER_H_
#define __GZIPSTREAMREADER_H_
#pragma once

#include <zlib.h>						// Include ZLIB library declarations
#include "Exception.h"					// Include Exception class declarations
#include "StreamReader.h"				// Include StreamReader declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// GZipStreamReader
//
// GZIP-based decompression stream reader implementation

class GZipStreamReader : public StreamReader
{
public:

	// Constructors / Destructor
	//
	GZipStreamReader(const void* base, size_t length);
	virtual ~GZipStreamReader();

	//-------------------------------------------------------------------------
	// Member Functions

	// StreamReader::Read
	//
	// Reads the specified number of bytes from the underlying stream
	virtual uint32_t Read(void* buffer, uint32_t length);

	// StreamReader::Reset
	//
	// Resets the stream back to the beginning
	virtual void Reset(void);

	// StreamReader::Seek
	//
	// Advances the stream to the specified position
	virtual void Seek(uint32_t position);

	//-------------------------------------------------------------------------
	// Properties

	// StreamReader::getPosition
	//
	// Gets the current position within the stream
	virtual uint32_t getPosition(void) { return m_position; }

private:

	GZipStreamReader(const GZipStreamReader&);
	GZipStreamReader& operator=(const GZipStreamReader&);

	//-------------------------------------------------------------------------
	// Member Variables

	z_stream				m_stream;			// GZIP decompression stream
	uint8_t*				m_base;				// Base memory address
	uint32_t				m_length;			// Length of memory buffer
	uint32_t				m_position;			// Current stream position
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __GZIPSTREAMREADER_H_
