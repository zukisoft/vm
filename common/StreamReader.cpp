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

#include "stdafx.h"
#include "StreamReader.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// StreamReader::TryRead
//
// Reads the specified number of bytes from the input stream into the output buffer.
// Returns a boolean success/failure value rather than throwing an exception
//
// Arguments:
//
//	buffer			- Output buffer
//	length			- Length of the output buffer, in bytes
//	out				- Number of bytes written to the buffer

bool StreamReader::TryRead(void* buffer, size_t length, size_t* out)
{
	if(!out) return false;					// Invalid [out] pointer

	try { *out = Read(buffer, length); return true; }
	catch(Exception&) { return false; }
}

//-----------------------------------------------------------------------------
// StreamReader::TrySeek
//
// Advances the stream to the specified position.  Returns a boolean value
// rather than throwing an exception
//
// Arguments:
//
//	position		- Position to advance the stream pointer to

bool StreamReader::TrySeek(size_t position)
{
	try { Seek(position); return true; }
	catch(Exception&) { return false; }
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
