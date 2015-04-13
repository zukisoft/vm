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
// PidNamespace::Allocate
//
// Allocates a unique pid_t from the pool
//
// Arguments:
//
//	NONE

uapi::pid_t PidNamespace::Allocate(void)
{
	uapi::pid_t value;			// Allocated pid_t value

	// Try to use a spent pid_t first before grabbing a new one; if the
	// return value overflowed, there are no more pids left to use
	if(!m_spent.try_pop(value)) value = m_next++;
	if(value < 0) throw Exception(E_FAIL);	// TODO: exception

	return value;
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
	return std::make_shared<PidNamespace>();
}

//-----------------------------------------------------------------------------
// PidNamespace::Release
//
// Releases a pid for re-use in the pool
//
// Arguments:
//
//	pid		- Spent pid_t to return to the pool

void PidNamespace::Release(uapi::pid_t pid)
{
	// This class reuses pids aggressively, push it into the spent queue
	// so that it be grabbed before allocating a new one
	m_spent.push(pid);	
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
