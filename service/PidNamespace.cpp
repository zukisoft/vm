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
#include "PidNamespace.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// PidNamespace Constructor (private)
//
// Arguments:
//
//	ancestor	- Ancestor PidNamespace instance

PidNamespace::PidNamespace(const std::shared_ptr<PidNamespace>& ancestor) : m_ancestor(ancestor)
{
}

//-----------------------------------------------------------------------------
// PidNamespace::Allocate (private)
//
// Allocates a unique pid_t from the available pool of values
//
// Arguments:
//
//	NONE

uapi::pid_t PidNamespace::Allocate(void)
{
	uapi::pid_t pid;				// Allocated pid_t value

	// Try to use a spent pid_t first before grabbing a new one; if the
	// return value overflows, there are no more pid_ts left to use
	if(!m_spent.try_pop(pid)) pid = m_next++;
	if(pid < 0) throw std::exception("todo");		// todo: exception

	return pid;
}

//-----------------------------------------------------------------------------
// PidNamespace::Create (static)
//
// Creates a new PidNamespace instance
//
// Arguments:
//
//	NONE

std::shared_ptr<PidNamespace> PidNamespace::Create(void)
{
	return Create(nullptr);		// Create a root PidNamespace
}

//-----------------------------------------------------------------------------
// PidNamespace::Create (static)
//
// Creates a new PidNamespace instance
//
// Arguments:
//
//	ancestor	- Ancestor PidNamespace instance

std::shared_ptr<PidNamespace> PidNamespace::Create(const std::shared_ptr<PidNamespace>& ancestor)
{
	return std::make_shared<PidNamespace>(ancestor);
}

//-----------------------------------------------------------------------------
// PidNamespace::CreatePid
//
// Allocates a new Pid instance.  The Pid will receive unique pid_t values for
// this namespace as well as any/all direct ancestor namespaces
//
// Arguments:
//
//	NONE

std::shared_ptr<Pid> PidNamespace::CreatePid(void)
{
	// Construct a new Pid instance and assign a pid_t from this namespace
	auto pid = std::make_shared<Pid>();
	pid->m_pids.emplace(shared_from_this(), Allocate());

	// Iterate over all ancestors to this namespace and assign pid_ts
	// from each of them to the Pid instance as well
	auto ancestor = m_ancestor.lock();
	while(ancestor) {

		pid->m_pids.emplace(ancestor, ancestor->Allocate());
		ancestor = ancestor->m_ancestor.lock();
	}

	return pid;
}

//-----------------------------------------------------------------------------
// PidNamespace::Release (private)
//
// Releases a previously allocated pid_t for reuse
//
// Arguments:
//
//	pid		- pid_t previous allocated by this instance

void PidNamespace::Release(uapi::pid_t pid)
{
	// This class reuses pid_ts aggressively, push it into the spent queue
	// so that it will be grabbed before allocating a new one
	m_spent.push(pid);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
