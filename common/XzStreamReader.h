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

#ifndef __XZSTREAMREADER_H_
#define __XZSTREAMREADER_H_
#pragma once

#include <xz.h>
#include "Exception.h"
#include "StreamReader.h"

#pragma warning(push, 4)				

//----------------------------------------------------------------------------
// EXTERNAL DEPENDENCY: XZ-EMBEDDED
//
// - Add the following files from external\xz-embedded to the parent project:
//
//	linux\include\linux\xz.h
//	linux\lib\xz\xz_crc32.c
//	linux\lib\xz\xz_dec_bcj.c
//	linux\lib\xz\xz_dec_lzma2.c
//	linux\lib\xz\xz_dec_stream.c
//
// - Disable precompiled headers for all the above .c files
// - Define XZ_DEC_X86 preprocessor definition for above .c files
// - Add external\xz-embedded\linux\include\linux and \external\xz-embedded\userspace
//   to the project Additional Include Directories
//
// NOTE: StreamReader functions limited to 32-bit lengths/positions since the
// XZ decoder library uses uint32_t internally and generates warnings on x64
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// XzStreamReader
//
// XZ-based decompression stream reader implementation

class XzStreamReader : public StreamReader
{
public:

	// Constructors / Destructor
	//
	XzStreamReader(const void* base, size_t length);
	virtual ~XzStreamReader();

	// StreamReader Implementation
	//
	virtual size_t	Read(void* buffer, size_t length);
	virtual void	Seek(size_t position);
	virtual size_t	getPosition(void) { return m_position; }

private:

	XzStreamReader(const XzStreamReader&)=delete;
	XzStreamReader& operator=(const XzStreamReader&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	xz_buf				m_buffer;				// XZ buffer structure
	xz_dec*				m_decoder;				// XZ decoder structure
	size_t				m_position = 0;			// Current position in the stream
	bool				m_finished = false;		// Flag for end of stream
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __XZSTREAMREADER_H_
