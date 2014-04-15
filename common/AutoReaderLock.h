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

#ifndef __AUTOREADERLOCK_H_
#define __AUTOREADERLOCK_H_
#pragma once

#include "ReaderWriterLock.h"		// Include ReaderWriterLock declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// AutoReaderLock
//
// Provides an automatic acquire/release wrapper around a slim reader lock

class AutoReaderLock
{
public:

	// Instance Constructor
	//
	explicit AutoReaderLock(ReaderWriterLock& lock) : m_lock(lock) { m_lock.AcquireReader(); }

	// Desructor
	~AutoReaderLock() { m_lock.ReleaseReader(); }

private:

	AutoReaderLock(const AutoReaderLock& rhs);
	AutoReaderLock& operator=(const AutoReaderLock& rhs);

	//-----------------------------------------------------------------------
	// Member Variables

	ReaderWriterLock&			m_lock;		// Referenced ReaderWriterLock
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif		// __AUTOREADERLOCK_H_
