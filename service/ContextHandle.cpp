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
#include "ContextHandle.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ContextHandle Constructor (private)
//
// Arguments:
//
//	vm			- VirtualMachine instance to associate with the context

ContextHandle::ContextHandle(const std::shared_ptr<::VirtualMachine>& vm) : m_vm(vm) 
{
	_ASSERTE(vm);
}

//-----------------------------------------------------------------------------
// ContextHandle Constructor (private)
//
// Arguments:
//
//	vm			- VirtualMachine instance to associate with the context
//	process		- Process instance to associate with the context

ContextHandle::ContextHandle(const std::shared_ptr<::VirtualMachine>& vm, const std::shared_ptr<::Process>& process) : m_vm(vm), m_process(process)
{
	_ASSERTE(vm);
	_ASSERTE(process);
}

//-----------------------------------------------------------------------------
// ContextHandle Constructor (private)
//
// Arguments:
//
//	vm			- VirtualMachine instance to associate with the context
//	process		- Process instance to associate with the context
//	thread		- Thread instance to associate with the context

ContextHandle::ContextHandle(const std::shared_ptr<::VirtualMachine>& vm, const std::shared_ptr<::Process>& process, const std::shared_ptr<::Thread>& thread)
	: m_vm(vm), m_process(process), m_thread(thread) 
{
	_ASSERTE(vm);
	_ASSERTE(process);
	_ASSERTE(thread);
}

//-----------------------------------------------------------------------------
// ContextHandle::Allocate (static)
//
// Allocates a new ContextHandle instance
//
// Arguments:
//
//	vm			- VirtualMachine instance to associate with the context

ContextHandle* ContextHandle::Allocate(const std::shared_ptr<::VirtualMachine>& vm)
{
	// Allocate the storage for the ContextHandle with MIDL_user_allocate
	void* instance = MIDL_user_allocate(sizeof(ContextHandle));
	if(!instance) throw Exception(E_OUTOFMEMORY);

	// Use placement new to construct the ContextHandle in the allocated storage
	return new(instance) ContextHandle(vm);
}

//-----------------------------------------------------------------------------
// ContextHandle::Allocate (static)
//
// Allocates a new ContextHandle instance
//
// Arguments:
//
//	vm			- VirtualMachine instance to associate with the context
//	process		- Process instance to associate with the context

ContextHandle* ContextHandle::Allocate(const std::shared_ptr<::VirtualMachine>& vm, const std::shared_ptr<::Process>& process)
{
	// Allocate the storage for the ContextHandle with MIDL_user_allocate
	void* instance = MIDL_user_allocate(sizeof(ContextHandle));
	if(!instance) throw Exception(E_OUTOFMEMORY);

	// Use placement new to construct the ContextHandle in the allocated storage
	return new(instance) ContextHandle(vm, process);
}

//-----------------------------------------------------------------------------
// ContextHandle::Allocate (static)
//
// Allocates a new ContextHandle instance
//
// Arguments:
//
//	vm			- VirtualMachine instance to associate with the context
//	process		- Process instance to associate with the context
//	thread		- Thread instance to associate with the context

ContextHandle* ContextHandle::Allocate(const std::shared_ptr<::VirtualMachine>& vm, const std::shared_ptr<::Process>& process, const std::shared_ptr<::Thread>& thread)
{
	// Allocate the storage for the ContextHandle with MIDL_user_allocate
	void* instance = MIDL_user_allocate(sizeof(ContextHandle));
	if(!instance) throw Exception(E_OUTOFMEMORY);

	// Use placement new to construct the ContextHandle in the allocated storage
	return new(instance) ContextHandle(vm, process, thread);
}

//-----------------------------------------------------------------------------
// ContextHandle::Release (static)
//
// Releases a ContextHandle instance
//
// Arguments:
//
//	context		- Existing ContextHandle instance

ContextHandle* ContextHandle::Release(ContextHandle* context)
{
	if(!context) return nullptr;

	context->~ContextHandle();		// Invoke object destructor
	MIDL_user_free(context);		// Release allocated storage
	return nullptr;					// Convenience NULL for the caller
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
