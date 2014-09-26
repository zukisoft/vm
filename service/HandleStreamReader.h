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

#ifndef __HANDLESTREAMREADER_H_
#define __HANDLESTREAMREADER_H_
#pragma once

#include <linux/fs.h>
#include "Exception.h"
#include "FileSystem.h"
#include "StreamReader.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// HandleStreamReader
//
// Implements a stream reader for a FileSystem::Handle instance, primarily
// used with internal virtual machine functions this is never exposed to the
// hosted applications

class HandleStreamReader : public StreamReader
{
public:

	// Constructor / Destructor
	//
	HandleStreamReader(const FileSystem::HandlePtr& handle) : m_handle(handle) {}
	virtual ~HandleStreamReader()=default;

	//---------------------------------------------------------------------
	// Properties

	// StreamReader Implementation
	virtual size_t	Read(void* buffer, size_t length);
	virtual void	Seek(size_t position);
	virtual size_t	getPosition(void) { return m_position; }

private:

	HandleStreamReader(const HandleStreamReader& rhs);
	HandleStreamReader& operator=(const HandleStreamReader& rhs);

	//-------------------------------------------------------------------------
	// Member Variables

	FileSystem::HandlePtr		m_handle;			// Handle instance reference
	size_t						m_position = 0;		// Current position
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HANDLESTREAMREADER_H_
