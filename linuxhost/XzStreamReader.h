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

#include "Exception.h"					// Include Exception class declarations
#include "StreamReader.h"				// Include StreamReader declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

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

	XzStreamReader(const XzStreamReader&);
	XzStreamReader& operator=(const XzStreamReader&);

	//-------------------------------------------------------------------------
	// Member Variables

	xz_buf					m_buffer;			// XZ buffer structure
	xz_dec*					m_decoder;			// XZ decoder structure

	uint8_t*				m_base;				// Base memory address
	uint32_t				m_length;			// Length of memory buffer
	uint32_t				m_position;			// Current position in the stream
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __XZSTREAMREADER_H_
