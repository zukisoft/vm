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

#include "stdafx.h"
#include "Host.h"

#pragma warning(push, 4)

// Host::ReadySignal::s_inheritflags
//
// Static SECURITY_ATTRIBUTES structure with bInheritHandle set to TRUE
SECURITY_ATTRIBUTES Host::ReadySignal::s_inherit = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

//-----------------------------------------------------------------------------
// Host Constructor
//
// Arguments:
//
//	procinfo		- PROCESS_INFORMATION for the hosted process

Host::Host(const PROCESS_INFORMATION& procinfo)
{
	m_procinfo = procinfo;
}

//-----------------------------------------------------------------------------
// Host Destructor

Host::~Host()
{
	// Close the process handles created along with the host process
	CloseHandle(m_procinfo.hThread);
	CloseHandle(m_procinfo.hProcess);
}

//-----------------------------------------------------------------------------
// Host::Create (static)
//
// Creates a new Host instance for a regular child process
//
// Arguments:
//
//	binarypath		- Path to the host binary
//	bindingstring	- RPC binding string to pass to the host binary
//	timeout			- Timeout value, in milliseconds, to wait for a host process

std::unique_ptr<Host> Host::Create(const tchar_t* binarypath, const tchar_t* bindingstring, DWORD timeout)
{
	zero_init<PROCESS_INFORMATION>	procinfo;			// Process information
	ReadySignal						readysignal;		// Event for child to signal

	// Generate the command line for the child process, which includes the RPC binding string as argv[1] and
	// a 32-bit serialized copy of the inheritable event handle as argv[2]
	tchar_t commandline[MAX_PATH];
	_sntprintf_s(commandline, MAX_PATH, MAX_PATH, _T("\"%s\" \"%s\" \"%ld\""), binarypath, bindingstring, static_cast<__int32>(readysignal));

	// TODO: If this technique will be used anywhere else, break out the PROCTHREADATTRIBUTE stuff into a class object;
	// this may not be a bad idea regardless, it's kind of ugly and makes this look more complicated than it is

	// Determine the size of the attributes buffer required to hold the inheritable handles property
	size_t required = 0;
	InitializeProcThreadAttributeList(nullptr, 1, 0, &required);
	if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) throw Win32Exception();

	// Allocate a buffer large enough to hold the attribute data and initialize it
	std::vector<uint8_t> buffer(required);
	PPROC_THREAD_ATTRIBUTE_LIST attributes = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(buffer.data());
	if(!InitializeProcThreadAttributeList(attributes, 1, 0, &required)) throw Win32Exception();

	try {

		// Add the ready event as in inheritable handle attribute for the client process
		HANDLE handles[] = { readysignal };
		if(!UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles, sizeof(handles), nullptr, nullptr)) throw Win32Exception();

		// Attempt to launch the process using the EXTENDED_STARTUP_INFO_PRESENT flag
		// TODO: NEEDS SECURITY SETTINGS AND OPTIONS
		zero_init<STARTUPINFOEX> startinfo;
		startinfo.StartupInfo.cb = sizeof(STARTUPINFOEX);
		startinfo.lpAttributeList = attributes;
		if(!CreateProcess(binarypath, commandline, nullptr, nullptr, TRUE, EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, 
			&startinfo.StartupInfo, &procinfo)) throw Win32Exception();

		DeleteProcThreadAttributeList(attributes);			// Clean up the PROC_THREAD_ATTRIBUTE_LIST
	}

	catch(...) { DeleteProcThreadAttributeList(attributes); throw; }

	// The process is alive, wait for it to either signal the ready event or to terminate
	HANDLE waithandles[] = { readysignal, procinfo.hProcess };
	DWORD wait = WaitForMultipleObjects(2, waithandles, FALSE, timeout);
	if(wait != WAIT_OBJECT_0) {

		// Process did not respond to the event, kill it if it's not already dead
		if(wait == WAIT_TIMEOUT) TerminateProcess(procinfo.hProcess, ERROR_TIMEOUT);

		// Close the process and thread handles returned from CreateProcess()
		CloseHandle(procinfo.hThread);
		CloseHandle(procinfo.hProcess);

		throw Exception(E_HOSTUNRESPONSIVE, binarypath);
	}

	// Process was successfully created and initialized, pass it off to a Host instance
	return std::make_unique<Host>(procinfo);
}

//-----------------------------------------------------------------------------

std::unique_ptr<Host> Host::TESTME(const tchar_t* binarypath, const tchar_t* bindingstring, DWORD timeout)
{
	zero_init<PROCESS_INFORMATION>	procinfo;			// Process information
	ReadySignal						readysignal;		// Event for child to signal

	// Generate the command line for the child process, which includes the RPC binding string as argv[1] and
	// a 32-bit serialized copy of the inheritable event handle as argv[2]
	tchar_t commandline[MAX_PATH];
	_sntprintf_s(commandline, MAX_PATH, MAX_PATH, _T("\"%s\" \"%s\" \"%ld\""), binarypath, bindingstring, static_cast<__int32>(readysignal));

	// TODO: If this technique will be used anywhere else, break out the PROCTHREADATTRIBUTE stuff into a class object;
	// this may not be a bad idea regardless, it's kind of ugly and makes this look more complicated than it is

	// Determine the size of the attributes buffer required to hold the inheritable handles property
	size_t required = 0;
	InitializeProcThreadAttributeList(nullptr, 1, 0, &required);
	if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) throw Win32Exception();

	// Allocate a buffer large enough to hold the attribute data and initialize it
	std::vector<uint8_t> buffer(required);
	PPROC_THREAD_ATTRIBUTE_LIST attributes = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(buffer.data());
	if(!InitializeProcThreadAttributeList(attributes, 1, 0, &required)) throw Win32Exception();

	try {

		// Add the ready event as in inheritable handle attribute for the client process
		HANDLE handles[] = { readysignal };
		if(!UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles, sizeof(handles), nullptr, nullptr)) throw Win32Exception();

		// Attempt to launch the process using the EXTENDED_STARTUP_INFO_PRESENT flag
		// TODO: NEEDS SECURITY SETTINGS AND OPTIONS
		// TEST: CREATE_SUSPENDED
		zero_init<STARTUPINFOEX> startinfo;
		startinfo.StartupInfo.cb = sizeof(STARTUPINFOEX);
		startinfo.lpAttributeList = attributes;
		if(!CreateProcess(binarypath, commandline, nullptr, nullptr, TRUE, CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, 
			&startinfo.StartupInfo, &procinfo)) throw Win32Exception();

		DeleteProcThreadAttributeList(attributes);			// Clean up the PROC_THREAD_ATTRIBUTE_LIST
	}

	catch(...) { DeleteProcThreadAttributeList(attributes); throw; }

	///////////////////////

	// testing VirtualAllocEx; do not run as Administrator to ensure this works as normal user

	// NOTE: can use a job object to group all processes together, that may be better to manage
	// when service dies killing all clients too.  Perhaps start the job with INIT rather than
	// the service, this mimicks userspace and the service can kill the entire job on PANIC

	void* testmem = VirtualAllocEx(procinfo.hProcess, nullptr, 8 * 1 MiB, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if(testmem) {

		BOOL result = VirtualFreeEx(procinfo.hProcess, testmem, 0, MEM_RELEASE);
		if(!result) {

			DWORD dw = GetLastError();
			int x = 123;
		}
	}
	else {

		DWORD dw = GetLastError();
		int x = 123;
	}

	ResumeThread(procinfo.hThread);

	// If anything above doesn't work, need to call TerminateProcess() to kill it otherwise it will
	// sit there suspended forever

	//////////////////////

	// The process is alive, wait for it to either signal the ready event or to terminate
	HANDLE waithandles[] = { readysignal, procinfo.hProcess };
	DWORD wait = WaitForMultipleObjects(2, waithandles, FALSE, timeout);
	if(wait != WAIT_OBJECT_0) {

		// Process did not respond to the event, kill it if it's not already dead
		if(wait == WAIT_TIMEOUT) TerminateProcess(procinfo.hProcess, ERROR_TIMEOUT);

		// Close the process and thread handles returned from CreateProcess()
		CloseHandle(procinfo.hThread);
		CloseHandle(procinfo.hProcess);

		throw Exception(E_HOSTUNRESPONSIVE, binarypath);
	}

	// Process was successfully created and initialized, pass it off to a Host instance
	return std::make_unique<Host>(procinfo);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
