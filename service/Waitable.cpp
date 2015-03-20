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

Waitable::Waitable() : m_state(State::NotSignaled)
{
	// Create a Win32 manual reset event to signal state changes
	m_statechanged = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if(!m_statechanged) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Waitable Destructor

Waitable::~Waitable()
{
	CloseHandle(m_statechanged);
}

//-----------------------------------------------------------------------------
// Waitable::PeekWaitableState
//
// Peeks the current waitable state without resetting the signal
//
// Arguments:
//
//	NONE

Waitable::State Waitable::PeekWaitableState(void)
{
	std::lock_guard<std::mutex> critsec(m_statelock);

	// Peeking the state just returns the current value
	return m_state;
}

//-----------------------------------------------------------------------------
// Waitable::PopWaitableState
//
// Pops the current waitable state and resets the signal
//
// Arguments:
//
//	NONE

Waitable::State Waitable::PopWaitableState(void)
{
	std::lock_guard<std::mutex> critsec(m_statelock);

	// Copy out the current state and reset it to NotSignaled
	State result = m_state;
	m_state = State::NotSignaled;

	// Reset the manual reset event object
	ResetEvent(m_statechanged);

	// Return the previously set state
	return result;
}

//-----------------------------------------------------------------------------
// Waitable::SetWaitableState (protected)
//
// Sets a waitable state for the object and signals the event
//
// Arguments:
//
//	state		- Waitable state to set for the object

void Waitable::SetWaitableState(State state)
{
	// NotSignaled should not actually be set as a new state, it indicates
	// that it has never been set or has been reset from a wait
	_ASSERTE(state != State::NotSignaled);

	std::lock_guard<std::mutex> critsec(m_statelock);

	// Save the new state code and signal the event; this will purposely lose 
	// any previously signaled state that was not retrieved
	m_state = state;
	SetEvent(m_statechanged);
}

//-----------------------------------------------------------------------------
// Waitable::getWaitableStateChanged
//
// Gets the Win32 event object handle used to signal state changes

HANDLE Waitable::getWaitableStateChanged(void) const
{
	return m_statechanged;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
