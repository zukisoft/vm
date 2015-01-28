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
#include "Thread.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Thread Constructor (private)
//
// Arguments:
//
//	tid			- Virtual thread identifier

Thread::Thread(uapi::pid_t tid) : m_tid(tid)
{
}

//-----------------------------------------------------------------------------
// Thread::getSignalMask
//
// Gets the current blocked signal mask for the thread

uapi::sigset_t Thread::getSignalMask(void) const
{
	return m_sigmask;
}

//-----------------------------------------------------------------------------
// Thread::putSignalMask
//
// Sets the blocked signal mask for the thread

void Thread::putSignalMask(uapi::sigset_t value)
{
	m_sigmask = value;
}

//-----------------------------------------------------------------------------
// Thread::getThreadId
//
// Gets the virtual thread identifier for this instance

uapi::pid_t Thread::getThreadId(void) const
{
	return m_tid;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
