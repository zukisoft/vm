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
#include <concurrent_priority_queue.h>
#include <memory>
#include <linux/signal.h>
#include "Architecture.h"
#include "NativeHandle.h"
#include "NtApi.h"
#include "TaskState.h"
#include "VirtualMachine.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Thread
//
// Represents a single thread within a hosted process instance

class Thread
{
public:

	// Destructor
	//
	~Thread()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// BeginSignal
	//
	// Begins queued execution of a signal handler on this thread
	void BeginSignal(int signal, uapi::sigaction action);

	// EndSignal
	//
	// Completes execution of the current signal handler on this thread
	void EndSignal(void);

	// FromNativeHandle
	//
	// Creates a new Thread instance from a native operating system handle
	template<Architecture architecture>
	static std::shared_ptr<Thread> FromNativeHandle(uapi::pid_t tid, const std::shared_ptr<::NativeHandle>& process, const std::shared_ptr<::NativeHandle>& thread, 
		DWORD threadid, std::unique_ptr<TaskState>&& initialtask);

	// PopInitialTask
	//
	// Pops the initial task information for the thread and clears it
	void PopInitialTask(void* task, size_t tasklength);

	// Resume
	//
	// Resumes the thread
	void Resume(void) const;

	// ResumeTask
	//
	// Resumes the thread after application of a task state
	void ResumeTask(const std::unique_ptr<TaskState>& initialtask) const;

	// SetSignalAlternateStack
	//
	// Sets the alternate stack to use for signal handlers
	void SetSignalAlternateStack(const uapi::stack_t* newstack, uapi::stack_t* oldstack);

	// SetSignalMask
	//
	// Sets the signal mask
	void SetSignalMask(const uapi::sigset_t* newmask, uapi::sigset_t* oldmask);

	// Suspend
	//
	// Suspends the thread
	void Suspend(void) const;

	// SuspendTask
	//
	// Suspends the thread and captures the task state
	std::unique_ptr<TaskState> SuspendTask(void);

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture flag for the thread instance
	__declspec(property(get=getArchitecture)) ::Architecture Architecture;
	::Architecture getArchitecture(void) const;

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

	// Process
	//
	// Gets a reference to the parent Process instance
	__declspec(property(get=getProcess)) std::shared_ptr<::Process> Process;
	std::shared_ptr<::Process> getProcess(void) const;

	// SignalAlternateStack
	//
	// Gets the alternate stack for signal handlers
	__declspec(property(get=getSignalAlternateStack)) uapi::stack_t SignalAlternateStack;
	uapi::stack_t getSignalAlternateStack(void) const;

	// SignalMask
	//
	// Gets/sets the blocked signal mask for the thread
	__declspec(property(get=getSignalMask)) uapi::sigset_t SignalMask;
	uapi::sigset_t getSignalMask(void) const;

	// ThreadId
	//
	// Gets the virtual thread identifier
	__declspec(property(get=getThreadId)) uapi::pid_t ThreadId;
	uapi::pid_t getThreadId(void) const;

	// TidAddress
	//
	// TODO: This is used for set_tid_address; needs to have a futex associated with 
	// it as well, stubbing this out so that sys_set_tid_address can be implemented
	__declspec(property(get=getTidAddress, put=putTidAddress)) void* TidAddress;
	void* getTidAddress(void) const { return m_tidaddress; }
	void putTidAddress(void* value) { m_tidaddress = value; }

	// Zombie
	//
	// Gets flag indicating if this thread is a 'zombie' or not
	__declspec(property(get=getZombie)) bool Zombie;
	bool getZombie(void) const;

private:

	Thread(const Thread&)=delete;
	Thread& operator=(const Thread&)=delete;

	// queued_signal_t
	//
	// Data type stored in the signal queue for this thread
	using queued_signal_t = std::pair<int, uapi::sigaction>;

	// rt_sigframe_x86
	//
	// 32-bit signal handler stack frame
	struct rt_sigframe_x86 {

		uint32_t				pretcode;
		int32_t					signal;
		uint32_t				pinfo;
		uint32_t				puc;
		// todo: siginfo			info;
		// todo: ucontext_ia32		uc;
		uint8_t					retcode[8];
		// todo: floating point state follows
	};

	// rt_sigframe_x64
	//
	// 64-bit signal handler stack frame
	struct rt_sigframe_x64 {

		uint64_t				pretcode;
		// todo: ucontext		uc;
		// todo: siginfo		info;
		// todo: floating point state follows
	};

	// sigframe_x86
	//
	// 32-bit signal handler stack frame
	struct sigframe_x86 {

		uint32_t				pretcode;
		int32_t					sig;
		// todo: sigcontext_ia32	sc;
		// todo: _fpstate_ia32		fpstate_unused;
		// todo: uint32_t			extramask[_NSIG_WORDS-1];
		uint8_t					retcode[8];
		// todo: floating point state follows
	};

	// signal_queue_predicate_t
	//
	// Predicate to use with signal_queue_t that defines the priority of the items.  Lower signal
	// ordinal values are given priority over higher ordinal values
	struct signal_queue_predicate_t : public std::binary_function<queued_signal_t, queued_signal_t, bool>
	{
		bool operator()(const queued_signal_t& lhs, const queued_signal_t& rhs) const { return lhs.first > rhs.first; }
	};

	// signal_queue_t
	//
	// Priority queue that holds the pending signals for this thread
	using signal_queue_t = Concurrency::concurrent_priority_queue<queued_signal_t, signal_queue_predicate_t>;

	// Instance Constructor
	//
	Thread(uapi::pid_t tid, ::Architecture architecture, const std::shared_ptr<::NativeHandle>& process, const std::shared_ptr<::NativeHandle>& thread, DWORD threadid,
		std::unique_ptr<TaskState>&& initialtask);
	friend class std::_Ref_count_obj<Thread>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ProcessQueuedSignal
	//
	// Processes a single signal popped from the queue
	void ProcessQueuedSignal(queued_signal_t signal);

	//-------------------------------------------------------------------------
	// Member Variables

	const uapi::pid_t				m_tid;				// Thread identifier
	const ::Architecture			m_architecture;		// Thread architecture
	std::shared_ptr<::NativeHandle>	m_process;			// Native process handle
	std::shared_ptr<::NativeHandle>	m_thread;			// Native thread handle
	const DWORD						m_threadid;			// Native thread id
	std::unique_ptr<TaskState>		m_initialtask;		// Initial task state

	void*					m_tidaddress = nullptr;

	// Signal Management
	//
	std::atomic<uapi::sigset_t>		m_sigmask = 0;		// Current signal mask
	std::atomic<uapi::stack_t>		m_sigaltstack;		// Alternate signal stack
	signal_queue_t					m_pendingsignals;	// Pending signals
	std::atomic<bool>				m_insignal = false;
	std::unique_ptr<TaskState>		m_savedsigtask;		// Previous task state
	uapi::sigset_t					m_savedsigmask;		// Previous signal mask
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __THREAD_H_
