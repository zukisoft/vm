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

#ifndef __THREAD_H_
#define __THREAD_H_
#pragma once

#include <atomic>
#include <memory>
#include <linux/signal.h>
#include "architecture_traits.h"
#include "Architecture.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// Thread
//
// Represents a single thread within a hosted process instance

class Thread
{
public:

	// Destructor
	//
	~Thread();

	//-------------------------------------------------------------------------
	// Member Functions

	// FromHandle
	//
	// Creates a new Thread instance from a native operating system handle
	template<Architecture architecture>
	static std::shared_ptr<Thread> FromHandle(uapi::pid_t tid, HANDLE nativehandle, DWORD nativetid);

	// Resume
	//
	// Resumes the thread
	void Resume(void);

	// SetSignalAlternateStack
	//
	// Sets the alternate stack to use for signal handlers
	void SetSignalAlternateStack(const uapi::stack_t* newstack, uapi::stack_t* oldstack);

	// Suspend
	//
	// Suspends the thread
	void Suspend(void);

	//-------------------------------------------------------------------------
	// Properties

	// NativeHandle
	//
	// Gets the native handle for the thread
	__declspec(property(get=getNativeHandle)) HANDLE NativeHandle;
	HANDLE getNativeHandle(void) const;

	// NativeThreadId
	//
	// Gets the native thread identifier
	__declspec(property(get=getNativeThreadId)) DWORD NativeThreadId;
	DWORD getNativeThreadId(void) const;

	// SignalAlternateStack
	//
	// Gets the alternate stack for signal handlers
	__declspec(property(get=getSignalAlternateStack, put=putSignalAlternateStack)) uapi::stack_t SignalAlternateStack;
	uapi::stack_t getSignalAlternateStack(void) const;

	// SignalMask
	//
	// Gets/sets the blocked signal mask for the thread
	__declspec(property(get=getSignalMask, put=putSignalMask)) uapi::sigset_t SignalMask;
	uapi::sigset_t getSignalMask(void) const;
	void putSignalMask(uapi::sigset_t value);

	// ThreadId
	//
	// Gets the virtual thread identifier
	__declspec(property(get=getThreadId)) uapi::pid_t ThreadId;
	uapi::pid_t getThreadId(void) const;

private:

	Thread(const Thread&)=delete;
	Thread& operator=(const Thread&)=delete;

	// Instance Constructor
	//
	Thread(Architecture architecture, uapi::pid_t tid, HANDLE nativehandle, DWORD nativetid);
	friend class std::_Ref_count_obj<Thread>;

	//-------------------------------------------------------------------------
	// Member Variables

	const Architecture			m_architecture;		// Thread architecture
	HANDLE						m_nativehandle;		// Native thread handle
	const DWORD					m_nativetid;		// Native thread identifier
	const uapi::pid_t			m_tid;				// Thread identifier
	std::atomic<uapi::sigset_t>	m_sigmask;			// Current signal mask
	std::atomic<uapi::stack_t>	m_sigaltstack;		// Alternate signal stack
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __THREAD_H_
