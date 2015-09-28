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

#ifndef __SYSTEMCALLCONTEXT_H_
#define __SYSTEMCALLCONTEXT_H_
#pragma once

#include <memory>

#pragma warning(push, 4)

// Forward Declarations
//
class Process;
class Thread;
class _VmOld;

//-----------------------------------------------------------------------------
// SystemCallContext
//
// Object type used as the RPC context handle for a client process; maintains
// references to various virtual machine objects that are used to implement
// the system calls.  Instances of this class must be created and destroyed
// with the provided static Allocate() and Release() methods

class SystemCallContext final
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates a new SystemCallContext instance
	static SystemCallContext* Allocate(std::shared_ptr<class _VmOld> vm);
	static SystemCallContext* Allocate(std::shared_ptr<class _VmOld> vm, std::shared_ptr<class Process> process);
	static SystemCallContext* Allocate(std::shared_ptr<class _VmOld> vm, std::shared_ptr<class Process> process, std::shared_ptr<class Thread> thread);

	// Release
	//
	// Releases a SystemCallContext instance
	static SystemCallContext* Release(SystemCallContext* context);

	//-------------------------------------------------------------------------
	// Fields

	// Process
	//
	// Process instance
	const std::shared_ptr<class Process> Process;

	// Thread
	//
	// Process thread instance
	const std::shared_ptr<class Thread> Thread;

	// _VmOld
	//
	// Virtual machine instance
	const std::shared_ptr<class _VmOld> _VmOld;

private:

	~SystemCallContext()=default;
	SystemCallContext(SystemCallContext const&)=delete;
	SystemCallContext& operator=(SystemCallContext const&)=delete;

	// Instance Constructor
	//
	SystemCallContext(std::shared_ptr<class _VmOld> vm, std::shared_ptr<class Process> process, std::shared_ptr<class Thread> thread);
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSTEMCALLCONTEXT_H_
