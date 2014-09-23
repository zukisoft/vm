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
#include "ElfImage.h"
#include "Exception.h"
#include "FileSystem.h"
#include "MemoryRegion.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// Class Host
//
// Manages the creation and lifetime of an ELF binary host child process

class Host
{
public:

	// Destructor
	//
	~Host();

	// Create (static)
	//
	// Creates a new Host instance for a regular child process
	// TODO: can get rid of the binarypath and timeout by passing in VmSettings instance -- do that instead
	static std::unique_ptr<Host> Create(const tchar_t* binarypath, const tchar_t* bindingstring, DWORD timeout);

	static std::unique_ptr<Host> TESTME(const FileSystem::HandlePtr handle, const tchar_t* binarypath, const tchar_t* bindingstring, DWORD timeout);

	//-------------------------------------------------------------------------
	// Properties

	// ProcessHandle
	//
	// Gets the host process handle
	__declspec(property(get=getProcessHandle)) HANDLE ProcessHandle;
	HANDLE getProcessHandle(void) const { return m_procinfo.hProcess; }

	// ProcessId
	//
	// Gets the host process identifier
	__declspec(property(get=getProcessId)) DWORD ProcessId;
	DWORD getProcessId(void) const { return m_procinfo.dwProcessId; }

private:

	Host(const Host&)=delete;
	Host& operator=(const Host&)=delete;

	// Instance Constructor
	//
	Host(const PROCESS_INFORMATION& procinfo);
	friend std::unique_ptr<Host> std::make_unique<Host, PROCESS_INFORMATION&>(PROCESS_INFORMATION&);

	// HandleStreamReader
	//
	// Implements a stream reader for a FileSystem::Handle instance
	class HandleStreamReader : public StreamReader
	{
	public:

		// Constructor / Destructor
		//
		HandleStreamReader(const FileSystem::HandlePtr& handle) : m_handle(handle) {}
		virtual ~HandleStreamReader()=default;

		//---------------------------------------------------------------------
		// Properties

		// StreamReader Implementation
		virtual size_t	Read(void* buffer, size_t length);
		virtual void	Seek(size_t position);
		virtual size_t	getPosition(void) { return m_position; }

	private:

		HandleStreamReader(const HandleStreamReader& rhs);
		HandleStreamReader& operator=(const HandleStreamReader& rhs);

		//-------------------------------------------------------------------------
		// Member Variables

		FileSystem::HandlePtr		m_handle;			// Handle instance reference
		size_t						m_position = 0;		// Current position
	};

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
#ifdef _M_X64
			// The raw value of the handle needs to fall within (INT32_MIN,INT32_MAX) for it to serialize properly
			__int64 serialized = reinterpret_cast<__int64>(m_handle);
			if((serialized > INT32_MAX) || (serialized < INT32_MIN)) throw Exception(E_CANNOTSERIALIZEHANDLE);
			else return static_cast<__int32>(serialized);
#else
			// 32-bit builds have 32-bit handles, serialization is not an issue
			return reinterpret_cast<__int32>(m_handle); 
#endif
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
