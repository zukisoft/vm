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

#ifndef __LZMASTREAMREADER_H_
#define __LZMASTREAMREADER_H_
#pragma once

#include <LzmaDec.h>
#include "Exception.h"
#include "StreamReader.h"

#pragma warning(push, 4)				

//----------------------------------------------------------------------------
// EXTERNAL DEPENDENCY: LZMA SDK
//
// - Add the following files from external\lzma\C to the parent project:
//
//	LzmaDec.h
//	LzmaDec.c
//	Types.h
//
// - Disable precompiled headers for all the above .c files
// - Add external\lmza\C to the project Additional Include Directories
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LzmaStreamReader
//
// LZMA-based decompression stream reader implementation

class LzmaStreamReader : public StreamReader
{
public:

	// Constructors / Destructor
	//
	LzmaStreamReader(const void* base, size_t length);
	virtual ~LzmaStreamReader();

	// StreamReader Implementation
	//
	virtual size_t	Read(void* buffer, size_t length);
	virtual void	Seek(size_t position);
	virtual size_t	getLength(void);
	virtual size_t	getPosition(void) { return m_position; }

private:

	LzmaStreamReader(const LzmaStreamReader&)=delete;
	LzmaStreamReader& operator=(const LzmaStreamReader&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	CLzmaDec				m_state;				// LZMA state structure
	uint64_t				m_streamlen;			// Uncompressed stream length
	uintptr_t				m_inputptr;				// Current input data pointer
	const uintptr_t			m_baseptr;				// Base memory address pointer
	const size_t			m_length;				// Length of memory buffer
	size_t					m_position = 0;			// Current position in the stream
	bool					m_finished = false;		// Flag for end of stream
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __LZMASTREAMREADER_H_
