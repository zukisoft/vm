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

	// Constructor / Destructor
	//
	BufferStreamReader(const void* base, size_t length);
	virtual ~BufferStreamReader()=default;

	// StreamReader Implementation
	//
	virtual size_t	Read(void* buffer, size_t length);
	virtual void	Seek(size_t position);
	virtual size_t	getLength(void) { return m_length; }
	virtual size_t	getPosition(void) { return m_offset; }

private:

	BufferStreamReader(const BufferStreamReader&)=delete;
	BufferStreamReader& operator=(const BufferStreamReader&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	const void*				m_base;				// Base memory address
	size_t					m_length;			// Length of memory buffer
	size_t					m_offset;			// Offset into the memory buffer
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __BUFFERSTREAMREADER_H_
