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

#ifndef __BUFFERSTREAMREADER_H_
#define __BUFFERSTREAMREADER_H_
#pragma once

#include "Exception.h"
#include "StreamReader.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// BufferStreamReader
//
// Memory buffer stream reader implementation

class BufferStreamReader : public StreamReader
{
public:

	// Constructors / Destructor
	//
	BufferStreamReader(const void* base, size_t length);
	virtual ~BufferStreamReader() {}

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
	// Gets the overall length of the stream data, if known
	virtual uint32_t getLength(void) { return m_length; }

	// StreamReader::getPosition
	//
	// Gets the current position within the stream
	virtual uint32_t getPosition(void) { return m_offset; }

private:

	BufferStreamReader(const BufferStreamReader&)=delete;
	BufferStreamReader& operator=(const BufferStreamReader&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	const void*				m_base;				// Base memory address
	uint32_t				m_length;			// Length of memory buffer
	uint32_t				m_offset;			// Offset into the memory buffer
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __BUFFERSTREAMREADER_H_
