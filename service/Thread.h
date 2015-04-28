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
#include <mutex>
#include <linux/resource.h>
#include <linux/signal.h>
#include <linux/wait.h>
#include "Architecture.h"
#include "LinuxException.h"
#include "NativeHandle.h"
#include "NtApi.h"
#include "TaskState.h"
#include "_VmOld.h"
#include "Win32Exception.h"

class Process;

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Thread
//
// Represents a single thread within a process/thread group

class Thread
{
public:

	// SignalResult
	//
	// Result provided by the thread from a signal request
	enum class SignalResult {

		Blocked			= 0,		// Signal is blocked by the thread
		Delivered,					// Signal was delivered to the thread
		Ignored,					// Signal was ignored by the thread
		CoreDump,					// Process should be core dumped
		Resume,						// Process should be resumed
		Suspend,					// Process should be suspended
		Terminate,					// Process should be terminated
	};

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

	// Exit
	//
	// Indicates that the thread terminated normally on its own
	void Exit(int exitcode);

	// GetResourceUsage
	//
	// Gets resource usage information for the thread
	void GetResourceUsage(int who, uapi::rusage* rusage);

	// FromNativeHandle
	//
	// Creates a new Thread instance from a native operating system handle
	//template<Architecture architecture>
	//static std::shared_ptr<Thread> FromNativeHandle(uapi::pid_t tid, const std::shared_ptr<::NativeHandle>& process, const std::shared_ptr<::NativeHandle>& thread, 
	//	DWORD threadid, std::unique_ptr<TaskState>&& initialtask);

	static std::shared_ptr<Thread> Create(const std::shared_ptr<::Process>& process, uapi::pid_t tid, const std::shared_ptr<NativeHandle>& thread, DWORD threadid, 
		std::unique_ptr<TaskState>&& task);

	// PopInitialTask
	//
	// Pops the initial task information for the thread and clears it
	void PopInitialTask(void* task, size_t tasklength);

	// Resume
	//
	// Resumes the thread
	void Resume(void);

	// SetSignalAlternateStack
	//
	// Sets the alternate stack to use for signal handlers
	void SetSignalAlternateStack(const uapi::stack_t* newstack, uapi::stack_t* oldstack);

	// SetSignalMask
	//
	// Sets the signal mask
	void SetSignalMask(const uapi::sigset_t* newmask, uapi::sigset_t* oldmask);

	// Signal
	//
	// Attempts to send a signal to this thread
	SignalResult Signal(int signal, uapi::sigaction action);

	// Start
	//
	// Starts the thread
	void Start(void);

	// Suspend
	//
	// Suspends the thread
	void Suspend(void);

	// Terminate
	//
	// Terminates the thread
	void Terminate(int exitcode);

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture flag for the thread instance
	__declspec(property(get=getArchitecture)) ::Architecture Architecture;
	::Architecture getArchitecture(void) const;

	// ClearThreadIdOnExit
	//
	// Gets/sets the address to be set to zero and signaled on normal thread exit
	__declspec(property(get=getClearThreadIdOnExit, put=putClearThreadIdOnExit)) void* ClearThreadIdOnExit;
	void* getClearThreadIdOnExit(void) const;
	void putClearThreadIdOnExit(void* value);

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
	Thread(uapi::pid_t tid, ::Architecture architecture, const std::shared_ptr<::Process>& process, const std::shared_ptr<::NativeHandle>& thread, 
		DWORD threadid, std::unique_ptr<TaskState>&& initialtask);
	friend class std::_Ref_count_obj<Thread>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// DefaultSignalResult (static)
	//
	// Determines the action to take for a defaulted signal
	static SignalResult DefaultSignalResult(int signal);
	
	// ProcessQueuedSignal
	//
	// Processes a single signal popped from the queue
	void ProcessQueuedSignal(queued_signal_t signal);

	// ResumeTask
	//
	// Resumes the thread, applying a new task state
	void ResumeTask(const std::unique_ptr<TaskState>& task);

	// SuspendTask
	//
	// Suspends the thread, capturing its task state
	std::unique_ptr<TaskState> SuspendTask(void);

	//-------------------------------------------------------------------------
	// Member Variables

	const uapi::pid_t					m_tid;				// Thread identifier
	const ::Architecture				m_architecture;		// Thread architecture
	std::shared_ptr<::Process>			m_process;			// Parent process
	std::shared_ptr<::NativeHandle>		m_thread;			// Native thread handle
	const DWORD							m_threadid;			// Native thread id
	std::unique_ptr<TaskState>			m_initialtask;		// Initial task state

	void*								m_cleartid = nullptr;

	// Signal Management
	//
	std::atomic<uapi::sigset_t>			m_sigmask = 0;		// Current signal mask
	std::atomic<uapi::stack_t>			m_sigaltstack;		// Alternate signal stack
	signal_queue_t						m_pendingsignals;	// Pending signals
	std::atomic<bool>					m_insignal = false;
	std::unique_ptr<TaskState>			m_savedsigtask;		// Previous task state
	uapi::sigset_t						m_savedsigmask;		// Previous signal mask
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __THREAD_H_
