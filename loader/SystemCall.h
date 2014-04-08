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

#ifndef __SYSTEMCALL_H_
#define __SYSTEMCALL_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// SystemCall
//
// Used to invoke a virtual system call directly.  The arguments are mapped into
// the proper registers of the CONTEXT structure in the expected order based on
// the current architecture

class SystemCall : public CONTEXT
{
public:

	// Instance Constructor
	//
	SystemCall(HMODULE module, uint32_t number) 
	{ 
		Eax = number;			// System call number should be in EAX

		// The system calls are accessed by ordinal in the DLL
		m_func = reinterpret_cast<SYSCALL>(GetProcAddress(module, reinterpret_cast<LPCSTR>(number)));
		if(m_func == nullptr) throw Exception(E_SYSCALLNOTFOUND, number);
	}

	//-------------------------------------------------------------------------
	// Member Functions

	// Invoke
	//
	// Invokes the specified system call
	int Invoke(void) { return m_func(this); }

	// Invoke (variadic)
	//
	// Recursively builds the arguments for the system call
	template <typename _first, typename... _remaining>
	int Invoke(const _first& first, const _remaining&... remaining) {

		switch(m_args) {

			case 0: this->Ebx = uintptr_t(first); break;
			case 1: this->Ecx = uintptr_t(first); break;
			case 2: this->Edx = uintptr_t(first); break;
			case 3: this->Esi = uintptr_t(first); break;
			case 4: this->Edi = uintptr_t(first); break;
			case 5: this->Ebx = uintptr_t(first); break;
			default: throw Exception(E_SYSCALLARGUMENTCOUNT, this->Eax);
		}

		// Increment the number of arguments so that the next one will
		// be placed into the correct register of the CONTEXT structure
		m_args++;

		// Recursively call Invoke(...) if there are arguments remaining,
		// otherwise the non-variadic overload will be called
		return Invoke(remaining...);
	}

private:

	SystemCall(const SystemCall& rhs);
	SystemCall& operator=(const SystemCall& rhs);

	//-------------------------------------------------------------------------
	// Private Type Declarations
	
	// SYSCALL
	//
	// Function pointer to a virtualized system call
	typedef int (*SYSCALL)(PCONTEXT context);

	//-------------------------------------------------------------------------
	// Member Variables
	
	size_t				m_args = 0;					// Number of arguments
	SYSCALL				m_func = nullptr;			// Function pointer
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSTEMCALL_H_
