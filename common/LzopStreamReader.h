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

#ifndef __LZOPSTREAMREADER_H_
#define __LZOPSTREAMREADER_H_
#pragma once

#include <minilzo.h>
#include "Exception.h"
#include "StreamReader.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

//----------------------------------------------------------------------------
// EXTERNAL DEPENDENCY: MINILZO
//
// - Add the following files from external\minilzo to the parent project:
//
//	lzoconf.h
//	lzodefs.h
//	minilzo.c
//	minilzo.h
//
// - Disable precompiled headers for all the above .c files
// - Add external\minilzo to the project Additional Include Directories
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// LzopStreamReader
//
// LZO-based decompression stream reader implementation

class LzopStreamReader : public StreamReader
{
public:

	// Constructors / Destructor
	//
	LzopStreamReader(const void* base, size_t length);
	virtual ~LzopStreamReader();

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

	// StreamReader::getPosition
	//
	// Gets the current position within the stream
	virtual uint32_t getPosition(void) { return m_position; }

private:

	LzopStreamReader(const LzopStreamReader&)=delete;
	LzopStreamReader& operator=(const LzopStreamReader&)=delete;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ReadNextBlock
	//
	// Reads the next block of compressed data from the input stream
	uint32_t ReadNextBlock(void);

	//-------------------------------------------------------------------------
	// Member Variables

	uint32_t				m_position = 0;		// Current stream position
	uint8_t*				m_block;			// Decompressed block data
	size_t					m_blocklen;			// Block data buffer size
	uint8_t*				m_blockcurrent;		// Pointer into block data
	uint32_t				m_blockremain;		// Remaining block data
	uint32_t				m_lzoflags;			// LZOP header flags
	intptr_t				m_lzopos;			// Position in LZO stream
	size_t					m_lzoremain;		// Remaining LZOP data
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __LZOPSTREAMREADER_H_
