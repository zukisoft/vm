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

#ifndef __BZIP2STREAMREADER_H_
#define __BZIP2STREAMREADER_H_
#pragma once

#include <bzlib.h>
#include "Exception.h"
#include "StreamReader.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// EXTERNAL DEPENDENCY: BIZP2
//
// - Add the following files from external\bzip2 to the parent project:
//
//	blocksort.c
//	bzcompress.c
//	bzlib.c
//	bzlib.h
//	crctable.c
//	decompress.c
//	huffman.c
//	randtable.c
//
// - Disable precompiled headers for all the above .c files
// - Define _CRT_SECURE_NO_WARNINGS and BZ_NO_STDIO preprocessor definitions for above .c files
// - Add external\bzip2 to the project Additional Include Directories
//
// - Add the following files from common to the parent project:
//
//	bz_internal_error.cpp
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// BZip2StreamReader
//
// BZIP2-based decompression stream reader implementation

class BZip2StreamReader : public StreamReader
{
public:

	// Constructors / Destructor
	//
	BZip2StreamReader(const void* base, size_t length);
	virtual ~BZip2StreamReader();

	// StreamReader Implementation
	//
	//
	virtual size_t	Read(void* buffer, size_t length);
	virtual void	Seek(size_t position);
	virtual size_t	getPosition(void) { return m_position; }

private:

	BZip2StreamReader(const BZip2StreamReader&)=delete;
	BZip2StreamReader& operator=(const BZip2StreamReader&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	bz_stream			m_stream;				// BZIP2 decompression stream
	size_t				m_position = 0;			// Current position in the stream
	bool				m_finished = false;		// End of stream has been reached
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __BZIP2STREAMREADER_H_
