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

#ifndef __HOST_H_
#define __HOST_H_
#pragma once

#include <memory>
#include <vector>
#include "Exception.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Class Host
//
// Manages the creation and lifetime of an ELF binary host child process

class Host
{
public:

	// Constructor / Destructor
	//
	Host(const PROCESS_INFORMATION& procinfo);
	~Host();

	// Create (static)
	//
	// Creates a new Host instance for a regular child process
	static std::unique_ptr<Host> Create(const tchar_t* binarypath, const tchar_t* bindingstring, DWORD timeout);

private:

	Host(const Host&)=delete;
	Host& operator=(const Host&)=delete;

	// ReadySignal
	//
	// Wraps a Win32 manual reset event that is passed to the client process
	// to signal that it has connected to the server successfully
	class ReadySignal
	{
	public:

		// Constructor / Destructor
		//
		ReadySignal() : m_handle(CreateEvent(&s_inherit, TRUE, FALSE, nullptr)) { if(!m_handle) throw Win32Exception(); }
		~ReadySignal() { CloseHandle(m_handle); }

		// HANDLE conversion operator
		//
		operator HANDLE() const { return m_handle; }

		// __int32 conversion operator
		//
		operator __int32() const 
		{ 
			// The raw value of the handle cannot exceed INT32_MAX for serialization
			if(reinterpret_cast<__int3264>(m_handle) > INT32_MAX) { /* TODO: EXCEPTION HERE */ }; 
			return reinterpret_cast<__int32>(m_handle); 
		}

		// Set
		//
		// Signals the event object
		void Set(void) { if(!SetEvent(m_handle)) throw Win32Exception(); }

	private:

		ReadySignal(const ReadySignal&)=delete;
		ReadySignal& operator=(const ReadySignal&)=delete;

		HANDLE						m_handle;	// Contained kernel event handle
		static SECURITY_ATTRIBUTES	s_inherit;	// Inheritable SECURITY_ATTRIBUTES
	};

	//-------------------------------------------------------------------------
	// Member Variables

	PROCESS_INFORMATION			m_procinfo;		// Process information
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOST_H_
