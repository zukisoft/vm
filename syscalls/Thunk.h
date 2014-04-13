//-----------------------------------------------------------------------------
// Copyright (c) 2014 Michael G. Brehm
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

#ifndef __THUNK_H_
#define __THUNK_H_
#pragma once

#include "Exception.h"					// Include Exception declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Thunk
//
// Used to invoke a virtual system call directly.  The arguments are mapped into
// the proper registers of the CONTEXT structure in the expected order based on
// the current architecture

class Thunk : public CONTEXT
{
public:

	// THUNK
	//
	// Function pointer to a virtualized system call
	typedef int (*THUNK)(PCONTEXT context);

	// Instance Constructor
	//
	Thunk(uint32_t number, THUNK syscall) : m_func(syscall) { Eax = number; }

	//-------------------------------------------------------------------------
	// Member Functions

	// Invoke
	//
	// Invokes the specified system call
	int Invoke(void) { m_args = 0; return m_func(this); }

	// Invoke (variadic)
	//
	// Recursively builds the arguments for the system call
	template <typename _first, typename... _remaining>
	int Invoke(const _first& first, const _remaining&... remaining) {

		// TODO: deal with more than five arguments; need a buffer to point
		// context->Esp to it as if it were a stack
		if(m_args > 5) _RPTF0(_CRT_ASSERT, "Thunk: Have not implemented stack arguments yet!");

		switch(m_args) {

			case 0: this->Ebx = uintptr_t(first); break;
			case 1: this->Ecx = uintptr_t(first); break;
			case 2: this->Edx = uintptr_t(first); break;
			case 3: this->Esi = uintptr_t(first); break;
			case 4: this->Edi = uintptr_t(first); break;
			case 5: this->Ebx = uintptr_t(first); break;
			default: throw Exception(E_UNEXPECTED);
		}

		// Increment the number of arguments so that the next one will
		// be placed into the correct register of the CONTEXT structure
		m_args++;

		// Recursively call Invoke(...) if there are arguments remaining,
		// otherwise the non-variadic overload will be called
		return Invoke(remaining...);
	}

private:

	Thunk(const Thunk& rhs);
	Thunk& operator=(const Thunk& rhs);

	//-------------------------------------------------------------------------
	// Member Variables
	
	size_t				m_args = 0;					// Number of arguments
	THUNK				m_func = nullptr;			// Function pointer
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __THUNK_H_
