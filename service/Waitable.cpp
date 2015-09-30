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
#include "Waitable.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Waitable Constructor (protected)
//
// Arguments:
//
//	NONE

Waitable::Waitable()
{
	// Initialize the pending signal information to all zeros
	memset(&m_pending, 0, sizeof(uapi::siginfo));
}

//-----------------------------------------------------------------------------
// Waitable::MaskAcceptsState (private)
//
// Arguments:
//
//	mask	- Wait operation flags and options mask
//	state	- State code to check against the provided mask

bool Waitable::MaskAcceptsState(int mask, State state)
{
	switch(state) {

		// WEXITED -- Accepts State::Exited, State::Killed, State::Dumped
		//
		case State::Exited:
		case State::Killed:
		case State::Dumped:
			return ((mask & LINUX_WEXITED) == LINUX_WEXITED);

		// WSTOPPED -- Accepts State::Trapped, State::Stopped
		//
		case State::Trapped:
		case State::Stopped:
			return ((mask & LINUX_WSTOPPED) == LINUX_WSTOPPED);

		// WCONTINUED -- Accepts State::Continued
		//
		case State::Continued:
			return ((mask & LINUX_WCONTINUED) == LINUX_WCONTINUED);
	}

	return false;
}

//-----------------------------------------------------------------------------
// Waitable::NotifyStateChange (protected)
//
// Signals that a change in waitable object state has occurred
//
// Arguments:
//
//	pid			- PID/TID of the object that has changed state
//	state		- New state to be reported for the object
//	status		- Current object status/exit code

void Waitable::NotifyStateChange(uapi::pid_t pid, State state, int32_t status)
{
	uapi::siginfo		siginfo;			// Signal information (SIGCHLD)

	// Convert the input arguments into a siginfo (SIGCHLD) structure
	siginfo.si_signo = LINUX_SIGCHLD;
	siginfo.si_errno = 0;
	siginfo.si_code = static_cast<int32_t>(state);
	siginfo.linux_si_pid = pid;
	siginfo.linux_si_uid = 0;
	siginfo.linux_si_status = status;

	// Lock the waiters collection and pending siginfo member variables
	std::lock_guard<std::mutex> cscollection(m_lock);

	// Revoke any pending signal that was not processed by a waiter
	memset(&m_pending, 0, sizeof(uapi::siginfo));

	for(auto& iterator : m_waiters) {

		// Take the lock for this waiter and check that it hasn't already been spent,
		// this is necessary to prevent a race condition wherein a second call to this
		// function before the waiter could be removed would resignal it
		std::unique_lock<std::mutex> cswaiter(iterator.lock);
		if(iterator.siginfo->linux_si_pid != 0) continue;

		// If this waiter is not interested in the signal, move on to the next one
		if(!MaskAcceptsState(iterator.options, state)) continue;

		// Write the signal information out through the provided pointer and assign
		// the context object of this waiter to the shared result object reference
		*iterator.siginfo = siginfo;
		iterator.result = iterator.object;

		// Signal the waiter's condition variable that a result is available
		iterator.signal.notify_one();

		// If WNOWAIT has not been specified by this waiter, the signal is consumed
		if((iterator.options & LINUX_WNOWAIT) == 0) return;
	}
	
	m_pending = siginfo;		// Save the signal for another waiter (WNOWAIT)
}

//-----------------------------------------------------------------------------
// Waitable::Wait (static)
//
// Waits for a Waitable instance to become signaled
//
// Arguments:
//
//	object		- Waitable instances to be waited upon
//	options		- Wait operation flags and options
//	siginfo		- On success, contains resultant signal information

std::shared_ptr<Waitable> Waitable::Wait(std::shared_ptr<Waitable> object, int options, uapi::siginfo* siginfo)
{
	std::vector<std::shared_ptr<Waitable>> waitvec;
	waitvec.emplace_back(std::move(object));

	return Wait(waitvec, options, siginfo);
}

//-----------------------------------------------------------------------------
// Waitable::Wait (static)
//
// Waits for a Waitable instance to become signaled
//
// Arguments:
//
//	objects		- Vector of Waitable instances to be waited upon
//	options		- Wait operation flags and options
//	siginfo		- On success, contains resultant signal information

std::shared_ptr<Waitable> Waitable::Wait(std::vector<std::shared_ptr<Waitable>> const& objects, int options, uapi::siginfo* siginfo)
{
	std::condition_variable			signal;				// Signaled on a successful wait
	std::mutex						lock;				// Condition variable synchronization object
	std::shared_ptr<Waitable>		signaled;			// Object that was signaled

	// The caller has to be willing to wait for something otherwise this would never return
	_ASSERTE(options & (LINUX_WEXITED | LINUX_WSTOPPED | LINUX_WCONTINUED));
	if(options & ~(LINUX_WEXITED | LINUX_WSTOPPED | LINUX_WCONTINUED)) throw LinuxException(LINUX_EINVAL);

	// The si_pid field of the signal information is used to detect spurious condition variable
	// wakes as well as preventing it from being signaled multiple times; initialize it to zero
	_ASSERTE(siginfo);
	siginfo->linux_si_pid = 0;

	// At least one Waitable instance must have been provided
	_ASSERTE(objects.size());
	if(objects.size() == 0) return nullptr;

	// Take the condition variable lock before adding any waiters
	std::unique_lock<std::mutex> cscondvar(lock);

	// Iterate over all of the Waitable instances to check for a pending signal that can be
	// consumed immediately, or to register a wait operation against it
	for(auto const& iterator : objects) {

		std::lock_guard<std::mutex> cswaitable(iterator->m_lock);

		// If there is already a pending state for this Waitable instance that matches the
		// requested wait operation mask, pull it out and stop registering waits
		if((iterator->m_pending.linux_si_pid != 0) && (MaskAcceptsState(options, static_cast<State>(iterator->m_pending.si_code)))) {

			// Pull out the pending signal information and assign the result object
			*siginfo = iterator->m_pending;
			signaled = iterator;

			// If WNOWAIT has not been specified, reset the pending signal information
			if((options & LINUX_WNOWAIT) == 0) memset(&iterator->m_pending, 0, sizeof(uapi::siginfo));

			signal.notify_one();			// Condition will signal immediately if waited upon
			break;							// No need to register any more waiters
		}

		// Unless WNOHANG has been specified, register a wait operation for this Waitable instance
		if((options & LINUX_WNOHANG) == 0) iterator->m_waiters.emplace_back(signal, lock, options, iterator, siginfo, signaled);
	}

	// If WNOHANG was specified, nothing will have been registered to wait against; only a
	// consumed pending signal from a Waitable instance is considered as a result
	if(options & LINUX_WNOHANG) return signaled;

	// Wait indefinitely for the condition variable to become signaled and retake the lock
	signal.wait(cscondvar, [&]() -> bool { return siginfo->linux_si_pid != 0; });

	// Remove this wait from the provided Waitable instances
	for(auto const& iterator : objects) {

		std::lock_guard<std::mutex> critsec(iterator->m_lock);
		iterator->m_waiters.remove_if([&](waiter_t const& item) -> bool { return &item.signal == &signal; });
	}

	return signaled;			// Return Waitable instance that was signaled
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
