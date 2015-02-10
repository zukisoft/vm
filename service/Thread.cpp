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

// Thread::FromHandle<Architecture::x86>
//
// Explicit Instantiation of template function
template std::shared_ptr<Thread> Thread::FromHandle<Architecture::x86>(uapi::pid_t, HANDLE, DWORD);

#ifdef _M_X64
// Thread::FromHandle<Architecture::x86_64>
//
// Explicit Instantiation of template function
template std::shared_ptr<Thread> Thread::FromHandle<Architecture::x86_64>(uapi::pid_t, HANDLE, DWORD);
#endif

//-----------------------------------------------------------------------------
// Thread Constructor (private)
//
// Arguments:
//
//	tid			- Virtual thread identifier

Thread::Thread(Architecture architecture, uapi::pid_t tid, HANDLE nativehandle, DWORD nativetid)
	: m_architecture(architecture), m_tid(tid), m_nativehandle(nativehandle), m_nativetid(nativetid),
	m_sigmask(0) // <-- needs to be passed in
{
	// The initial alternate signal handler stack is disabled
	m_sigaltstack = { nullptr, LINUX_SS_DISABLE, 0 };
}

//-----------------------------------------------------------------------------
// Thread Destructor

Thread::~Thread()
{
	CloseHandle(m_nativehandle);
}

//-----------------------------------------------------------------------------
// Thread::FromHandle (static)
//
// Arguments:
//
//	tid				- Thread ID to assign to the thread
//	nativehandle	- Native operating system handle
//	nativetid		- Native operating system thread identifier

template<Architecture architecture>
std::shared_ptr<Thread> Thread::FromHandle(uapi::pid_t tid, HANDLE nativehandle, DWORD nativetid)
{
	return std::make_shared<Thread>(architecture, tid, nativehandle, nativetid);
}

//-----------------------------------------------------------------------------
// Thread::getNativeHandle
//
// Gets the native operating system handle for this thread

HANDLE Thread::getNativeHandle(void) const
{
	return m_nativehandle;
}

//-----------------------------------------------------------------------------
// Thread::getNativeThreadId
//
// Gets the native operating thread identifier

DWORD Thread::getNativeThreadId(void) const
{
	return m_nativetid;
}

//-----------------------------------------------------------------------------
// Thread::Resume
//
// Resumes the thread from a suspended state
//
// Arguments:
//
//	NONE

void Thread::Resume(void)
{
	if(ResumeThread(m_nativehandle) == -1) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Thread::SetSignalAlternateStack
//
// Sets the alternate signal handler stack information
//
// Arguments:
//
//	newstack		- Optional new stack information
//	oldstack		- Optional old stack information

void Thread::SetSignalAlternateStack(const uapi::stack_t* newstack, uapi::stack_t* oldstack)
{
	// If the original stack information is requested, copy that out first
	if(oldstack) *oldstack = m_sigaltstack;

	// If new stack information is provided, change the contained stack information
	if(newstack) {

		// Verify the pointer does not exceed the allowable maximum address
		if(m_architecture == Architecture::x86) { architecture_traits<Architecture::x86>::CheckPointer(newstack->ss_sp); }
		m_sigaltstack = *newstack;
	}
}

//-----------------------------------------------------------------------------
// Thread::SetSignalMask
//
// Sets the signal mask for the thread
//
// Arguments:
//
//	newmask			- Optional new signal mask
//	oldmask			- Optional old signal mask

void Thread::SetSignalMask(const uapi::sigset_t* newmask, uapi::sigset_t* oldmask)
{
	// If the original mask is requested, copy that out first
	if(oldmask) *oldmask = m_sigmask;

	// If a new mask is provided, change the contained information but always
	// ensure that SIGKILL and SIGSTOP are set, these signals cannot be masked
	if(newmask) m_sigmask = (*newmask | uapi::sigmask(LINUX_SIGKILL) | uapi::sigmask(LINUX_SIGSTOP));
}

//-----------------------------------------------------------------------------
// Thread::getSignalAlternateStack
//
// Gets the alternate signal handler stack information

uapi::stack_t Thread::getSignalAlternateStack(void) const
{
	return m_sigaltstack;
}

//-----------------------------------------------------------------------------
// Thread::getSignalMask
//
// Gets the current signal mask for the thread

uapi::sigset_t Thread::getSignalMask(void) const
{
	return m_sigmask;
}

//-----------------------------------------------------------------------------
// Thread::Suspend
//
// Suspends the thread
//
// Arguments:
//
//	NONE

void Thread::Suspend(void)
{
#ifndef _M_X64

	if(SuspendThread(m_nativehandle) == -1) throw Win32Exception();

#else

	// On 64-bit builds, Wow64SuspendThread() should be used for 32-bit threads
	if(m_architecture == Architecture::x86) { if(Wow64SuspendThread(m_nativehandle) == -1) throw Win32Exception(); }
	else if(SuspendThread(m_nativehandle) == -1) throw Win32Exception();

#endif
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
