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

#include "stdafx.h"
#include "SystemCallContext.h"

#include "Exception.h"
#include "Process.h"
#include "Thread.h"
#include "_VmOld.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SystemCallContext Constructor (private)
//
// Arguments:
//
//	vm			- _VmOld instance to associate with the context
//	process		- Process instance to associate with the context
//	thread		- Thread instance to associate with the context

SystemCallContext::SystemCallContext(std::shared_ptr<class _VmOld> vm, std::shared_ptr<class Process> process, std::shared_ptr<class Thread> thread)
	: _VmOld{ std::move(vm) }, Process{ std::move(process) }, Thread{ std::move(thread) }
{
}

//-----------------------------------------------------------------------------
// SystemCallContext::Allocate (static)
//
// Allocates a new SystemCallContext instance
//
// Arguments:
//
//	vm			- _VmOld instance to associate with the context

SystemCallContext* SystemCallContext::Allocate(std::shared_ptr<class _VmOld> vm)
{
	return Allocate(std::move(vm), nullptr, nullptr);
}

//-----------------------------------------------------------------------------
// SystemCallContext::Allocate (static)
//
// Allocates a new SystemCallContext instance
//
// Arguments:
//
//	vm			- _VmOld instance to associate with the context
//	process		- Process instance to associate with the context

SystemCallContext* SystemCallContext::Allocate(std::shared_ptr<class _VmOld> vm, std::shared_ptr<class Process> process)
{
	return Allocate(std::move(vm), std::move(process), nullptr);
}

//-----------------------------------------------------------------------------
// SystemCallContext::Allocate (static)
//
// Allocates a new SystemCallContext instance
//
// Arguments:
//
//	vm			- _VmOld instance to associate with the context
//	process		- Process instance to associate with the context
//	thread		- Thread instance to associate with the context

SystemCallContext* SystemCallContext::Allocate(std::shared_ptr<class _VmOld> vm, std::shared_ptr<class Process> process, std::shared_ptr<class Thread> thread)
{
	// Allocate the storage for the SystemCallContext with MIDL_user_allocate
	void* instance = MIDL_user_allocate(sizeof(SystemCallContext));
	if(!instance) throw Exception{ E_OUTOFMEMORY };

	// Use placement new to construct the SystemCallContext in the allocated storage
	return new(instance) SystemCallContext{ std::move(vm), std::move(process), std::move(thread) };
}

//-----------------------------------------------------------------------------
// SystemCallContext::Release (static)
//
// Releases a SystemCallContext instance
//
// Arguments:
//
//	context		- Existing SystemCallContext instance

SystemCallContext* SystemCallContext::Release(SystemCallContext* context)
{
	if(!context) return nullptr;

	context->~SystemCallContext();			// Invoke object destructor
	MIDL_user_free(context);		// Release allocated storage
	return nullptr;					// Convenience NULL for the caller
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
