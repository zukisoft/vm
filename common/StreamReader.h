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

#ifndef __STREAMREADER_H_
#define __STREAMREADER_H_
#pragma once

#include "Exception.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// StreamReader
//
// Implements a forward-only byte stream reader interface

class __declspec(novtable) StreamReader
{
public:

	// Destructor
	//
	virtual ~StreamReader()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Read
	//
	// Reads the specified number of bytes from the underlying stream
	virtual size_t Read(void* buffer, size_t length) = 0;

	// Seek
	//
	// Advances the stream to the specified position
	virtual void Seek(size_t position) = 0;

	// TryRead
	//
	// Reads the specified number of bytes from the underlying stream
	// Returns boolean success/failure rather than throwing an exception
	virtual bool TryRead(void* buffer, size_t length, size_t* out);

	// TrySeek
	//
	// Advances the stream to the specified position, returns a boolean
	// success/failure rather than throwing an exception
	virtual bool TrySeek(size_t position);

	//-------------------------------------------------------------------------
	// Properties

	// Length
	//
	// Gets the overall length of the stream, if known
	__declspec(property(get=getLength)) size_t Length;
	virtual size_t getLength(void) const { return MAXSIZE_T; }

	// Position
	//
	// Gets the current position within the stream
	__declspec(property(get=getPosition)) size_t Position;
	virtual size_t getPosition(void) = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __STREAMREADER_H_
