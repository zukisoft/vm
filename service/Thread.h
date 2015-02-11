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
#include "NtApi.h"
#include "TaskState.h"
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
	~Thread();

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

	// FromHandle
	//
	// Creates a new Thread instance from a native operating system handle
	// todo: Remove processtemp
	template<Architecture architecture>
	static std::shared_ptr<Thread> FromHandle(HANDLE processtemp, uapi::pid_t tid, HANDLE nativehandle, DWORD nativetid);

	// Resume
	//
	// Resumes the thread
	void Resume(void);

	// ResumeTask
	//
	// Resumes the thread after application of a task state
	void ResumeTask(const std::unique_ptr<TaskState>& task);

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
	void Suspend(void);

	// SuspendTask
	//
	// Suspends the thread and captures the task state
	std::unique_ptr<TaskState> SuspendTask(void);

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
	Thread(Architecture architecture, HANDLE processtemp, uapi::pid_t tid, HANDLE nativehandle, DWORD nativetid);
	friend class std::_Ref_count_obj<Thread>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ProcessQueuedSignal
	//
	// Processes a single signal popped from the queue
	void ProcessQueuedSignal(queued_signal_t signal);

	//-------------------------------------------------------------------------
	// Member Variables

	// temporary? may want weak_pointer to the Process instead here
	HANDLE						m_processtemp;

	const Architecture			m_architecture;		// Thread architecture
	HANDLE						m_nativehandle;		// Native thread handle
	const DWORD					m_nativetid;		// Native thread identifier
	const uapi::pid_t			m_tid;				// Thread identifier
	std::atomic<uapi::sigset_t>	m_sigmask;			// Current signal mask
	std::atomic<uapi::stack_t>	m_sigaltstack;		// Alternate signal stack

	// Signal Management
	//
	signal_queue_t				m_pendingsignals;	// Pending signals
	std::atomic<bool>			m_insignal = false;
	std::unique_ptr<TaskState>	m_savedsigtask;		// Previous task state
	uapi::sigset_t				m_savedsigmask;		// Previous signal mask
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __THREAD_H_
