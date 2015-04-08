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
#include "Process.h"

#pragma warning(push, 4)

//// Thread::FromNativeHandle<Architecture::x86>
////
//// Explicit Instantiation of template function
//template std::shared_ptr<Thread> Thread::FromNativeHandle<Architecture::x86>(uapi::pid_t pid, const std::shared_ptr<::NativeHandle>& process, 
//	const std::shared_ptr<::NativeHandle>& thread, DWORD threadid, std::unique_ptr<TaskState>&& task);
//
//#ifdef _M_X64
//// Thread::FromNativeHandle<Architecture::x86_64>
////
//// Explicit Instantiation of template function
//template std::shared_ptr<Thread> Thread::FromNativeHandle<Architecture::x86_64>(uapi::pid_t pid, const std::shared_ptr<::NativeHandle>& process, 
//	const std::shared_ptr<::NativeHandle>& thread, DWORD threadid, std::unique_ptr<TaskState>&& task);
//#endif

//-----------------------------------------------------------------------------
// Thread Constructor (private)
//
// Arguments:
//
//	vm				- Reference to the parent VirtualMachine instance
//	tid				- Virtual thread identifier
//	architecture	- Process/thread architecture
//	process			- Parent process handle
//	thread			- Native thread handle
//	threadid		- Native thread identifier
//	initialtask		- Initial thread task information

Thread::Thread(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t tid, ::Architecture architecture, const std::shared_ptr<::Process>& process, 
	const std::shared_ptr<::NativeHandle>& thread, DWORD threadid, std::unique_ptr<TaskState>&& initialtask) : m_vm(vm), m_tid(tid), m_architecture(architecture), 
	m_process(process), m_thread(thread), m_threadid(threadid), m_initialtask(std::move(initialtask))
{
	// The initial alternate signal handler stack is disabled
	m_sigaltstack = { nullptr, LINUX_SS_DISABLE, 0 };
}

//-----------------------------------------------------------------------------
// Thread Destructor

Thread::~Thread()
{
	// Release the thread TID if this is not the thread group leader (having a
	// TID that matches the parent process PID).
	if(m_tid != m_process->ProcessId) m_vm->ReleasePID(m_tid);
}

//-----------------------------------------------------------------------------
// Thread::getArchitecture
//
// Gets the process architecture type

::Architecture Thread::getArchitecture(void) const
{
	return m_architecture;
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

//-----------------------------------------------------------------------------
// Thread::getClearThreadIdOnExit
//
// Gets the address of a thread id to clear and signal when the thread exits

void* Thread::getClearThreadIdOnExit(void) const
{
	return m_cleartid;
}

//-----------------------------------------------------------------------------
// Thread::putClearThreadIdOnExit
//
// Sets the address of a thread id to clear and signal when the thread exits

void Thread::putClearThreadIdOnExit(void* value)
{
	m_cleartid = value;
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

	// If x64, the linux kernel decrements the stack pointer by 128 bytes as a 'red zone'

	// check for possible stack overflows and set stack pointer to a bad place to cause
	// an exception

	// x86 can have 2 different stack frames, one for sigaction and one for rt_sigaction
	// x64 only has rt_sigaction stack frame
	//
	// the data in these stack frames is crazy big -- probably want a whole class for that
	// like 'SignalStackFrame' (Architecture specific)

	// looks like those sneaky linux guys actually tuck the state away on the stack,
	// which is actually quite a good idea and saves trouble here if that's the right
	// context

	// start a signal handler callback
	auto newstate = TaskState::Duplicate(m_savedsigtask);
	
	// signal number in E/RAX
	newstate->ReturnValue = signal.first;

	// instruction pointer
	newstate->InstructionPointer = signal.second.sa_handler;

	sigframe_x86 frame;
	frame.sig = signal.first;
	frame.pretcode = reinterpret_cast<uint32_t>(&signal.second.sa_restorer);

	// retcode
	// the embedded pointer to sigreturn can't be specified - no idea where sys_sigreturn will be in the host
	// pop eax; mov eax, XXXXXXXXh; int 80h
	//frame.retcode = { 0x58, 0xB8, 0x00, 0x00, 0x00, 0x00, 0xCD, 0x80 };

	// rt_retcode
	// the embedded pointer to sigreturn can't be specified - no idea where sys_rt_sigreturn will be in the host
	// mov eax, XXXXXXXXh; int 80h;
	//frame.retcode = { 0xB8, 0x00, 0x00, 0x00, 0x00, 0x80, 0xCD, 0x00 };

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
	// edit: it's actually just __cdecl:
	//
	//	RETURN ADDRESS
	//	ARG1			- SIGNAL NUMBER
	//	ARG2			- SIGINFO*
	//	ARG3			- UCONTEXT*
	//  ... more stuff follows
	uint32_t signo = signal.first;
	stackpointer -= sizeof(uint32_t);
	//NtApi::NtWriteVirtualMemory(m_process->Handle, reinterpret_cast<void*>(stackpointer), &signo, sizeof(uint32_t), nullptr);
	m_process->WriteMemory(reinterpret_cast<void*>(stackpointer), &signo, sizeof(uint32_t));
	
	if(signal.second.sa_flags & LINUX_SA_RESTORER) {

		// write the sa_restorer pointer to the stack
		stackpointer -= sizeof(uint32_t);
		m_process->WriteMemory(reinterpret_cast<void*>(stackpointer), &signal.second.sa_restorer, sizeof(uint32_t));
		//NtApi::NtWriteVirtualMemory(m_process->Handle, reinterpret_cast<void*>(stackpointer), &signal.second.sa_restorer, sizeof(uint32_t), nullptr);
	}
	
	newstate->StackPointer = reinterpret_cast<void*>(stackpointer);

	ResumeTask(newstate);
	
	// in theory this ultimately calls into EndSignal from the remote thread
	// which undoes what was done here
}

//-----------------------------------------------------------------------------
// Thread::DefaultSignalResult (private, static)
//
// Determines the action to take for a defaulted signal
//
// Arguments:
//
//	signal		- Signal to be defauted

Thread::SignalResult Thread::DefaultSignalResult(int signal)
{
	// Defaults as defined in the signal(7) man page ...
	//
	switch(signal) {

		case LINUX_SIGHUP:		return SignalResult::Terminate;
		case LINUX_SIGINT:		return SignalResult::Terminate;
		case LINUX_SIGQUIT:		return SignalResult::CoreDump;
		case LINUX_SIGILL:		return SignalResult::CoreDump;
		case LINUX_SIGTRAP:		return SignalResult::CoreDump;
		case LINUX_SIGABRT:		return SignalResult::CoreDump;
		case LINUX_SIGBUS:		return SignalResult::CoreDump;
		case LINUX_SIGFPE:		return SignalResult::CoreDump;
		case LINUX_SIGKILL:		return SignalResult::Terminate;
		case LINUX_SIGUSR1:		return SignalResult::Terminate;
		case LINUX_SIGSEGV:		return SignalResult::CoreDump;
		case LINUX_SIGUSR2:		return SignalResult::Terminate;
		case LINUX_SIGPIPE:		return SignalResult::Terminate;
		case LINUX_SIGALRM:		return SignalResult::Terminate;
		case LINUX_SIGTERM:		return SignalResult::Terminate;
		case LINUX_SIGSTKFLT:	return SignalResult::Terminate;
		case LINUX_SIGCHLD:		return SignalResult::Ignored;
		case LINUX_SIGCONT:		return SignalResult::Resume;
		case LINUX_SIGSTOP:		return SignalResult::Suspend;
		case LINUX_SIGTSTP:		return SignalResult::Suspend;
		case LINUX_SIGTTIN:		return SignalResult::Suspend;
		case LINUX_SIGTTOU:		return SignalResult::Suspend;
		case LINUX_SIGURG:		return SignalResult::Ignored;
		case LINUX_SIGXCPU:		return SignalResult::CoreDump;
		case LINUX_SIGXFSZ:		return SignalResult::CoreDump;
		case LINUX_SIGVTALRM:	return SignalResult::Terminate;
		case LINUX_SIGPROF:		return SignalResult::Terminate;
		case LINUX_SIGWINCH:	return SignalResult::Ignored;
		case LINUX_SIGIO:		return SignalResult::Terminate;
		case LINUX_SIGPWR:		return SignalResult::Terminate;
		case LINUX_SIGSYS:		return SignalResult::CoreDump;
	}

	// Default result for any other signal is TERM
	return SignalResult::Terminate;
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
	//
	// *IF* everything needed is stored on the stack when sigaled, this becomes easier
	// as we can just grab it from there and reapply it rather than storing everything

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
// Thread::Exit
//
// Indicates that the thread terminated normally on its own
//
// Arguments:
//
//	exitcode		- Thread exit code

void Thread::Exit(int exitcode)
{
	// TODO: BLOCK ALL SIGNALS AND CLEAR ANY PENDING STATES

	// All this does is signal anything waiting on StateChanged event that
	// the thread has died, do not wait for it since its technically still
	// running and has native termination code that has to execute

	// todo: what should be done with the exit code
	(exitcode);
}

//-----------------------------------------------------------------------------
// Thread::FromNativeHandle (static)
//
// Arguments:
//
//	tid				- Virtual thread identifier
//	process			- Parent process handle
//	thread			- Native thread handle
//	threadid		- Native thread identifier
//	initialtask		- Initial task for the thread

//template<Architecture architecture>
//std::shared_ptr<Thread> Thread::FromNativeHandle(uapi::pid_t tid, const std::shared_ptr<::NativeHandle>& process, const std::shared_ptr<::NativeHandle>& thread, 
//	DWORD threadid, std::unique_ptr<TaskState>&& initialtask)
//{
//	return std::make_shared<Thread>(tid, architecture, process, thread, threadid, std::move(initialtask));
//}

std::shared_ptr<Thread> Thread::Create(const std::shared_ptr<VirtualMachine>& vm, const std::shared_ptr<::Process>& process, uapi::pid_t tid, 
	const std::shared_ptr<::NativeHandle>& thread, DWORD threadid, std::unique_ptr<TaskState>&& initialtask)
{
	return std::make_shared<Thread>(vm, tid, process->Architecture, process, thread, threadid, std::move(initialtask));
}

//-----------------------------------------------------------------------------
// Thread::GetResourceUsage
//
// Gets resource usage information for the thread
//
// Arguments:
//
//	who		- Flag indicating what usage information to collect
//	rusage	- Resultant resource usage information structure

void Thread::GetResourceUsage(int who, uapi::rusage* rusage)
{
	FILETIME	creation, exit, kernel, user;		// Information from GetThreadTimes()

	// RUSAGE_THREAD must be specified
	_ASSERTE(who == LINUX_RUSAGE_THREAD);
	if(who != LINUX_RUSAGE_THREAD) throw LinuxException(LINUX_EINVAL);

	// Initialize the entire structure to zero, most of the fields won't be populated
	_ASSERTE(rusage);
	memset(rusage, 0, sizeof(uapi::rusage));

	// There may be more that can be acquired via undocumented NTAPI functions, but for now I believe
	// only the system/user times are available for a specific thread
	if(!GetThreadTimes(m_thread->Handle, &creation, &exit, &kernel, &user)) throw Win32Exception();

	// Convert the total kernel time into uapi::timeval
	uint64_t totalkernel = ((static_cast<uint64_t>(kernel.dwHighDateTime) << 32) + kernel.dwLowDateTime) / 10;
	rusage->ru_systime.tv_sec = totalkernel / 1000000;			// Seconds
	rusage->ru_systime.tv_usec = totalkernel % 1000000;			// Microseconds

	// Convert the total user time into uapi::timeval
	uint64_t totaluser = ((static_cast<uint64_t>(user.dwHighDateTime) << 32) + user.dwLowDateTime) / 10;
	rusage->ru_utime.tv_sec = totaluser / 1000000;				// Seconds
	rusage->ru_utime.tv_usec = totaluser % 1000000;				// Microseconds
}

//-----------------------------------------------------------------------------
// Thread::getNativeHandle
//
// Gets the native operating system handle for this thread

HANDLE Thread::getNativeHandle(void) const
{
	return m_thread->Handle;
}

//-----------------------------------------------------------------------------
// Thread::getNativeThreadId
//
// Gets the native operating thread identifier

DWORD Thread::getNativeThreadId(void) const
{
	return m_threadid;
}

//-----------------------------------------------------------------------------
// Thread::PopInitialTask
//
// Pops the initial thread task information. This is only accessed once when
// the thread is getting itself ready to run, therefore the data is released
// after that occurs
//
// Arguments:
//
//	task		- Buffer to receive the task information
//	tasklen		- Length of the buffer to receive the task information

void Thread::PopInitialTask(void* task, size_t tasklen)
{
	if(!m_initialtask) throw Exception(E_FAIL);	// todo: Exception

	if(tasklen != m_initialtask->Length) throw Exception(E_FAIL); // todo: exception
	memcpy(task, m_initialtask->Data, m_initialtask->Length);

	// The task state for the thread can only be accessed one time
	m_initialtask.reset(nullptr);
}

//-----------------------------------------------------------------------------
// Thread::Resume
//
// Resumes the thread
//
// Arguments:
//
//	NONE

void Thread::Resume(void)
{
	if(ResumeThread(m_thread->Handle) == -1) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Thread::ResumeTask (private)
//
// Resumes the thread with a new task state
//
// Arguments:
//
//	task		- Task state to be applied to the thread

void Thread::ResumeTask(const std::unique_ptr<TaskState>& task)
{
	_ASSERTE(task);

	// Apply the specified task state to the thread
	task->Restore(m_architecture, m_thread->Handle);

	// Resume the thread
	Resume();
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
// Thread::Signal
//
// Attempts to send a signal to this thread
//
// Arguments:
//
//	signal		- Signal to be processed by this thread
//	action		- Action defined for the signal

Thread::SignalResult Thread::Signal(int signal, uapi::sigaction action)
{
	// TODO: check mask, is the thread alive, etc
	(action);
	(signal);

	// TODO: return true if signal will be processed, false if not
	return SignalResult::Blocked;
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
// Thread::Start
//
// Starts the thread
//
// Arguments:
//
//	NONE

void Thread::Start(void)
{
	Resume();
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
	// 32-bit builds use SuspendThread() exclusively
	if(SuspendThread(m_thread->Handle) == -1) throw Win32Exception();
#else
	// On 64-bit builds, Wow64SuspendThread() should be used for 32-bit threads
	if(m_architecture == Architecture::x86) { if(Wow64SuspendThread(m_nativehandle) == -1) throw Win32Exception(); }
	else if(SuspendThread(m_nativehandle) == -1) throw Win32Exception();
#endif
}

//-----------------------------------------------------------------------------
// Thread::SuspendTask (private)
//
// Suspends the thread, capturing its current task state
//
// Arguments:
//
//	NONE

std::unique_ptr<TaskState> Thread::SuspendTask(void)
{
	Suspend();					// Execute a regular Suspend() first

	// Attempt to capture the task state for the thread; resume on exception
	try { return TaskState::Capture(m_architecture, m_thread->Handle); }
	catch(...) { Resume(); throw; }
}

//-----------------------------------------------------------------------------
// Thread::Terminate
//
// Terminates the thread
//
// Arguments:
//
//	exitcode		- Exit code for the thread

void Thread::Terminate(int exitcode)
{
	// Terminate the thread and wait for it to actually terminate
	TerminateThread(m_thread->Handle, exitcode);
	WaitForSingleObject(m_thread->Handle, INFINITE);

	// todo: what should be done with the exit code now?
	// todo: need to do all the things Exit() needs to do; what good is Exit() going to be?
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
