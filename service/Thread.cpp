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
// Thread::BeginSignal
//
// Begins execution of a signal handler on the thread
//
// Arguments:
//
//	signal			- Signal to be processed
//	action			- Action to be taken for the signal

void Thread::BeginSignal(int signal, uapi::sigaction action)
{
	// IF THE SPECIFIED SIGNAL IS NOT ALREADY PENDING AND NOT SA_NODEFER
	// THEN PUSH INTO THE QUEUE
	m_pendingsignals.push(std::make_pair(signal, action));
	
	// EVERYTHING ELSE IN HERE WILL MOVE TO THAT QUEUE HANDLER
	// perhaps use a condition variable -- or a thread pool?

	// make it so DEFAULT cannot be passed in, this should always have a sigaction
	// that can be operated upon.  Even if it's just to terminate the thread/process.
	// "SignalActions" class can hold static default sigactions for everything

	// Define the signal mask to use while the handler is executing; this is the
	// combination of the current mask, the action mask and the signal being handled
	uapi::sigset_t mask = m_sigmask | action.sa_mask;
	if((action.sa_flags & LINUX_SA_NODEFER) == 0) mask |= uapi::sigmask(signal);

	// SAVE THE MASK AND TASK STATE
	m_savedsigtask = SuspendTask();

	// create the new task state
	// resumetask() the thread

	// return to caller, but the saved state must be popped by sigreturn() ? how would this
	// work for non-x86 architectures, or is sigreturn() always always called?
}

//-----------------------------------------------------------------------------
// Thread::EndSignal
//
// Completes execution of a signal handler on the thread
//
// Arguments:
//
//	NONE

void Thread::EndSignal(void)
{
	// here I guess we just need to restore the saved signal state, but
	// if there are other signals pending there is no point in restoring the
	// task state, that should be reapplied only when all signals are done?
	// the signal mask should be retrieved/restored in-between signals, though.
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
// Thread::ResumeTask
//
// Resumes the thread from a suspended state after applying a task state
//
// Arguments:
//
//	task		- Task state to be applied to the thread

void Thread::ResumeTask(const std::unique_ptr<TaskState>& task)
{
	// Apply the specified task to the native thread
	task->ToNativeThread(m_architecture, m_nativehandle);

	// Resume the thread
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

#ifdef _M_X64
		// Verify the pointer does not exceed the allowable maximum address
		if((m_architecture == Architecture::x86) && (uintptr_t(newstack->ss_sp) > UINT32_MAX)) throw Exception(E_THREADINVALIDSIGALTSTACK);
#endif
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
// Thread::SuspendTask
//
// Suspends the thread and captures it's task state
//
// Arguments:
//
//	NONE

std::unique_ptr<TaskState> Thread::SuspendTask(void)
{
	Suspend();						// Suspend the native operating system thread
	
	// Attempt to capture the task state for the thread, and resume on exception
	try { return TaskState::FromNativeThread(m_architecture, m_nativehandle); }
	catch(...) { Resume(); throw; }
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
