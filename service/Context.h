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

#ifndef __CONTEXT_H_
#define __CONTEXT_H_
#pragma once

#include "Process.h"
#include "Thread.h"
#include "_VmOld.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Context
//
// Object type used as the RPC context handle for a client process; maintains
// references to various virtual machine objects that are used to implement
// the system calls.  Instances of this class must be created and destroyed
// with the provided static Allocate() and Release() methods

class Context
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates a new Context instance
	static Context* Allocate(const std::shared_ptr<::_VmOld>& vm);
	static Context* Allocate(const std::shared_ptr<::_VmOld>& vm, const std::shared_ptr<::Process>& process);
	static Context* Allocate(const std::shared_ptr<::_VmOld>& vm, const std::shared_ptr<::Process>& process, const std::shared_ptr<::Thread>& thread);

	// Release
	//
	// Releases a Context instance
	static Context* Release(Context* context);

	//-------------------------------------------------------------------------
	// Fields

	// Process
	//
	// Virtual machine process instance
	const std::shared_ptr<::Process> Process;

	// Thread
	//
	// Process thread instance
	const std::shared_ptr<::Thread> Thread;

	// _VmOld
	//
	// Virtual machine instance
	const std::shared_ptr<::_VmOld> _VmOld;

private:

	~Context()=default;
	Context(const Context&)=delete;
	Context& operator=(const Context&)=delete;

	// Instance Constructors
	//
	Context(const std::shared_ptr<::_VmOld>& vm);
	Context(const std::shared_ptr<::_VmOld>& vm, const std::shared_ptr<::Process>& process);
	Context(const std::shared_ptr<::_VmOld>& vm, const std::shared_ptr<::Process>& process, const std::shared_ptr<::Thread>& thread);
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __CONTEXT_H_
