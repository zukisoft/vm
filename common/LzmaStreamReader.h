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

#ifndef __LZMASTREAMREADER_H_
#define __LZMASTREAMREADER_H_
#pragma once

#include <LzmaDec.h>
#include "Exception.h"
#include "StreamReader.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

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

	//-------------------------------------------------------------------------
	// Member Functions

	// StreamReader::Read
	//
	// Reads the specified number of bytes from the underlying stream
	virtual uint32_t Read(void* buffer, uint32_t length);

	// StreamReader::Seek
	//
	// Advances the stream to the specified position
	virtual void Seek(uint32_t position);

	//-------------------------------------------------------------------------
	// Properties

	// StreamReader::getLength
	//
	// Gets the length of the data stream, if known
	virtual uint32_t getLength(void) { return (m_streamlen < UINT32_MAX) ? static_cast<uint32_t>(m_streamlen) : UINT32_MAX; }

	// StreamReader::getPosition
	//
	// Gets the current position within the stream
	virtual uint32_t getPosition(void) { return m_position; }

private:

	LzmaStreamReader(const LzmaStreamReader&)=delete;
	LzmaStreamReader& operator=(const LzmaStreamReader&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	CLzmaDec				m_state;				// LZMA state structure
	uint64_t				m_streamlen;			// Uncompressed stream length
	uintptr_t				m_inputptr;				// Current input data pointer
	const uintptr_t			m_baseptr;				// Base memory address pointer
	const uint32_t			m_length;				// Length of memory buffer
	uint32_t				m_position = 0;			// Current position in the stream
	bool					m_finished = false;		// Flag for end of stream
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __LZMASTREAMREADER_H_
