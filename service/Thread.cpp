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
template std::shared_ptr<Thread> Thread::FromHandle<Architecture::x86>(HANDLE, uapi::pid_t, HANDLE, DWORD);

#ifdef _M_X64
// Thread::FromHandle<Architecture::x86_64>
//
// Explicit Instantiation of template function
template std::shared_ptr<Thread> Thread::FromHandle<Architecture::x86_64>(HANDLE, uapi::pid_t, HANDLE, DWORD);
#endif

//-----------------------------------------------------------------------------
// Thread Constructor (private)
//
// Arguments:
//
//	tid			- Virtual thread identifier

Thread::Thread(Architecture architecture, HANDLE processtemp, uapi::pid_t tid, HANDLE nativehandle, DWORD nativetid)
	: m_architecture(architecture), m_tid(tid), m_nativehandle(nativehandle), m_nativetid(nativetid),
	m_sigmask(0), m_processtemp(processtemp) // <-- needs to be passed in
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
	// TODO: RULES -- check the mask, don't queue multiples, etc
	// There is also a rule for the maximum # of queued signals (32, I think).
	// Consider a signal handler that never returns or calls EndSignal
	m_pendingsignals.push(std::make_pair(signal, action));

	// Only process a pending signal if not already handling one; the next
	// one will be popped from the queue when that signal finishes
	bool expected = false;
	if(m_insignal.compare_exchange_strong(expected, true)) {

		queued_signal_t signal;
		if(m_pendingsignals.try_pop(signal)) ProcessQueuedSignal(signal);
		else m_insignal = false;
	}
}

// WIP
//
// The goal here is to never actually block the calling RPC thread and not have any
// OS handles or things that need to be cleaned up if the thread dies.  If a thread
// dies in the middle of a signal handler or enters an infinite loop, that shouldn't
// matter to this code.
//
// there is also a race condition to deal with when a thread (or process) is first
// created, until it's done with acquiring context and getting itself set up, it 
// isn't eligible to be pre-empted by a signal at all as the necessary stuff like
// the ldt or the GS register may not be set yet.
//
// and to make it even harder, have to deal with what happens if this is invoked while
// the thread is in a system call (RPC in our case), it needs to be restarted or
// killed, not just pick up where it left off (which may not work anyway). easy, huh?
//
// the RPC call for sigreturn() -- can it be set up such that it just fires and then
// the calling client thread is suspended?  It's not supposed to return and although
// it will end up blowing the stack away, what happens to the RPC call and its memory?
void Thread::ProcessQueuedSignal(queued_signal_t signal)
{
	_ASSERTE(m_insignal);				// Should only be called if "in signal"

	// mask
	uapi::sigset_t mask = m_sigmask | signal.second.sa_mask;
	if((signal.second.sa_flags & LINUX_SA_NODEFER) == 0) mask |= uapi::sigmask(signal.first);
	m_savedsigmask = m_sigmask;
	m_sigmask = mask;

	// task state
	m_savedsigtask = SuspendTask();

	//
	// TODO: DEFAULT HANDLERS AND WHATNOT HERE
	//

	// start a signal handler callback
	auto newstate = m_savedsigtask->Duplicate();
	
	// signal number in E/RAX
	newstate->AX = signal.first;

	// instruction pointer
	newstate->InstructionPointer = signal.second.sa_handler;

	// stack pointer
	// todo: needs to be aligned - see align_sigframe in signal.c
	uintptr_t stackpointer = uintptr_t(newstate->StackPointer);
	if((signal.second.sa_flags & LINUX_SA_ONSTACK) && (m_sigaltstack.operator uapi::stack_t().ss_flags != LINUX_SS_DISABLE)) {

		stackpointer = uintptr_t(m_sigaltstack.operator uapi::stack_t().ss_sp);
	}

	// TODO: bunch of stuff needs to be on the stack here for compatibility
	// see arch\x86\kernel\signal.c
	// don't forget x86/x64 are going to be different sizes here

	// TESTING: push signal number onto the stack first?
	uint32_t signo = signal.first;
	stackpointer -= sizeof(uint32_t);
	NtApi::NtWriteVirtualMemory(m_processtemp, reinterpret_cast<void*>(stackpointer), &signo, sizeof(uint32_t), nullptr);
	
	if(signal.second.sa_flags & LINUX_SA_RESTORER) {

		// write the sa_restorer pointer to the stack
		stackpointer -= sizeof(uint32_t);
		NtApi::NtWriteVirtualMemory(m_processtemp, reinterpret_cast<void*>(stackpointer), &signal.second.sa_restorer, sizeof(uint32_t), nullptr);
	}
	
	newstate->StackPointer = reinterpret_cast<void*>(stackpointer);

	ResumeTask(newstate);
	
	// in theory this ultimately calls into EndSignal from the remote thread
	// which undoes what was done here
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
	// the signal mask should be retrieved/restored in-between signals, though

	_ASSERTE(m_insignal);
	if(!m_insignal) return;

	Suspend();

	// TODO: need to kick off additional signals here
	//queued_signal_t signal;
	//if(m_pendingsignals.try_pop(signal)) {

	//	ProcessQueuedSignal(signal);
	//	return;
	//}

	m_sigmask = m_savedsigmask;
	ResumeTask(m_savedsigtask);
	
	m_insignal = false;
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
std::shared_ptr<Thread> Thread::FromHandle(HANDLE processtemp, uapi::pid_t tid, HANDLE nativehandle, DWORD nativetid)
{
	return std::make_shared<Thread>(architecture, processtemp, tid, nativehandle, nativetid);
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
