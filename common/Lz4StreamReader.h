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

#ifndef __LZ4STREAMREADER_H_
#define __LZ4STREAMREADER_H_
#pragma once

#include <lz4.h>
#include "Exception.h"
#include "StreamReader.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// EXTERNAL DEPENDENCY: LZ4
//
// - Add the following files from external\lz4 to the parent project:
//
//	lz4.c
//	lz4.h
//
// - Disable precompiled headers for all the above .c files
// - Add external\lz4 to the project Additional Include Directories
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Lz4StreamReader
//
// LZ4-based decompression stream reader implementation

class Lz4StreamReader : public StreamReader
{
public:

	// Constructors / Destructor
	//
	Lz4StreamReader(const void* base, size_t length);
	virtual ~Lz4StreamReader();

	// StreamReader Implementation
	//
	virtual size_t	Read(void* buffer, size_t length);
	virtual void	Seek(size_t position);
	virtual size_t	getPosition(void) { return m_position; }

private:

	Lz4StreamReader(const Lz4StreamReader&)=delete;
	Lz4StreamReader& operator=(const Lz4StreamReader&)=delete;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ReadNextBlock
	//
	// Reads the next block of compressed data from the input stream
	uint32_t ReadNextBlock(void);

	//-------------------------------------------------------------------------
	// Member Variables

	size_t					m_position = 0;		// Current stream position
	uint8_t*				m_block;			// Decompressed block data
	uint8_t*				m_blockcurrent;		// Pointer into block data
	uint32_t				m_blockremain;		// Remaining block data
	intptr_t				m_lz4pos;			// Position in LZ4 stream
	size_t					m_lz4remain;		// Remaining LZ4 data
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __LZ4STREAMREADER_H_
