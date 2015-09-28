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
#include "Context.h"

#include "Exception.h"

#pragma warning(push, 4)

// Context::t_instance
//
// Thread-local Context instance
thread_local Context* Context::t_instance = nullptr;

//-----------------------------------------------------------------------------
// Context Constructor
//
// Arguments:
//
//	uid			- User id to associate with the context
//	gid			- Group id to associate with the context

Context::Context(uapi::pid_t uid, uapi::pid_t gid) : m_uid(uid), m_gid(gid)
{
}

//-----------------------------------------------------------------------------
// Context::Acquire (static)
//
// Retrieves the context instance for the current thread
//
// Arguments:
//
//	NONE

Context* Context::Acquire(void)
{
	_ASSERTE(t_instance);
	if(t_instance == nullptr) throw Exception{ E_FAIL };	// todo: custom exception

	return t_instance;
}

//-----------------------------------------------------------------------------
// Context::Attach (static)
//
// Attaches a context instance to the current thread
//
// Arguments:
//
//	context		- Context object to associate with the current thread

void Context::Attach(Context* context)
{
	_ASSERTE(t_instance == nullptr);
	if(t_instance) throw Exception{ E_FAIL };	// todo: custom exception

	t_instance = context;
}

//-----------------------------------------------------------------------------
// Context::Detach (static)
//
// Detaches the context from the current thread
//
// Arguments:
//
//	NONE

Context* Context::Detach(void)
{
	_ASSERTE(t_instance);
	
	Context* previous = t_instance;
	t_instance = nullptr;

	return previous;
}

//-----------------------------------------------------------------------------
// Context::getGroupId
//
// Gets the group id associated with this thread

uapi::pid_t Context::getGroupId(void) const
{
	return m_gid;
}

//-----------------------------------------------------------------------------
// Context::getUserId
//
// Gets the user id associated with this thread

uapi::pid_t Context::getUserId(void) const
{
	return m_uid;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
