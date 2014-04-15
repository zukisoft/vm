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

#ifndef __STREAMREADER_H_
#define __STREAMREADER_H_
#pragma once

#include "Exception.h"					// Include Exception declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// StreamReader
//
// Implements a forward-only byte stream reader interface

class StreamReader
{
public:

	// Destructor
	//
	virtual ~StreamReader() {}

	//-------------------------------------------------------------------------
	// Member Functions

	// Read
	//
	// Reads the specified number of bytes from the underlying stream
	virtual uint32_t Read(void* buffer, uint32_t length) = 0;

	// Reset
	//
	// Resets the stream back to the beginning
	virtual void Reset(void) = 0;

	// Seek
	//
	// Advances the stream to the specified position
	virtual void Seek(uint32_t position) = 0;

	// TryRead
	//
	// Reads the specified number of bytes from the underlying stream
	// Returns boolean success/failure rather than throwing an exception
	virtual bool TryRead(void* buffer, uint32_t length, uint32_t* out);

	// TrySeek
	//
	// Advances the stream to the specified position, returns a boolean
	// success/failure rather than throwing an exception
	virtual bool TrySeek(uint32_t position);

	//-------------------------------------------------------------------------
	// Properties

	// Position
	//
	// Gets the current position within the stream
	__declspec(property(get=getPosition)) uint32_t Position;
	virtual uint32_t getPosition(void) = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __STREAMREADER_H_
