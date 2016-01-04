//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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

#ifndef __CONTEXT_H_
#define __CONTEXT_H_
#pragma once

#pragma warning(push, 4)

// Forward Declarations
//

//-----------------------------------------------------------------------------
// Context
//
// Provides contextual information about the current thread of execution within
// the virtual machine, such as the current user and group, current capabilities
// bitmask and so on

class Context
{
public:

	// Instance Constructor
	//
	Context(uapi::pid_t uid, uapi::pid_t gid);

	// Destructor
	//
	virtual ~Context()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Acquire
	//
	// Acquires a pointer to the Context object for the current thread
	static Context* Acquire(void);

	// Attach (static)
	//
	// Attaches a Context instance to the current thread
	static void Attach(Context* context);

	// Detach (static)
	//
	// Detaches the context instance from the current thread
	static Context* Detach(void);

	//-------------------------------------------------------------------------
	// Properties

	// GroupId
	//
	// Gets the group id associated with this thread
	__declspec(property(get=getGroupId)) uapi::pid_t GroupId;
	uapi::pid_t getGroupId(void) const;

	// UserId
	//
	// Gets the user id associated with this thread
	__declspec(property(get=getUserId)) uapi::pid_t UserId;
	uapi::pid_t getUserId(void) const;

private:

	Context(Context const&)=delete;
	Context& operator=(Context const&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	thread_local static Context*	t_instance;		// Thread-local instance

	uapi::pid_t						m_uid;			// UID
	uapi::pid_t						m_gid;			// GID
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __CONTEXT_H_
