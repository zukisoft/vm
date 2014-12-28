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
#include "Host.h"

#pragma warning(push, 4)

// Host::NtResumeProcess
//
Host::NtResumeProcessFunc Host::NtResumeProcess = 
reinterpret_cast<NtResumeProcessFunc>([]() -> FARPROC {
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "NtResumeProcess");
}());

// Host::NtSuspendProces
//
Host::NtSuspendProcessFunc Host::NtSuspendProcess = 
reinterpret_cast<NtSuspendProcessFunc>([]() -> FARPROC {
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "NtSuspendProcess");
}());

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
//	path			- Path to the host binary
//	arguments		- Arguments to pass to the host binary
//	handles			- Optional array of inheritable handle objects
//	numhandles		- Number of elements in the handles array

std::unique_ptr<Host> Host::Create(const tchar_t* path, const tchar_t* arguments, HANDLE handles[], size_t numhandles)
{
	PROCESS_INFORMATION				procinfo;			// Process information

	// If a null argument string was provided, change it to an empty string
	if(arguments == nullptr) arguments = _T("");

	// Generate the command line for the child process, using the specifed path as argument zero
	tchar_t commandline[MAX_PATH];
	_sntprintf_s(commandline, MAX_PATH, MAX_PATH, _T("\"%s\"%s%s"), path, (arguments[0]) ? _T(" ") : _T(""), arguments);

	// Determine the size of the attributes buffer required to hold the inheritable handles property
	SIZE_T required = 0;
	InitializeProcThreadAttributeList(nullptr, 1, 0, &required);
	if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) throw Win32Exception();

	// Allocate a buffer large enough to hold the attribute data and initialize it
	HeapBuffer<uint8_t> buffer(required);
	PPROC_THREAD_ATTRIBUTE_LIST attributes = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(&buffer);
	if(!InitializeProcThreadAttributeList(attributes, 1, 0, &required)) throw Win32Exception();

	try {

		// UpdateProcThreadAttribute will fail if there are no handles in the specified array
		if((handles != nullptr) && (numhandles > 0)) {
			
			// Add the array of handles as inheritable handles for the client process
			if(!UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles, numhandles * sizeof(HANDLE), 
				nullptr, nullptr)) throw Win32Exception();
		}

		// Attempt to launch the process using the CREATE_SUSPENDED and EXTENDED_STARTUP_INFO_PRESENT flag
		zero_init<STARTUPINFOEX> startinfo;
		startinfo.StartupInfo.cb = sizeof(STARTUPINFOEX);
		startinfo.lpAttributeList = attributes;
		if(!CreateProcess(path, commandline, nullptr, nullptr, TRUE, CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT, nullptr, 
			nullptr, &startinfo.StartupInfo, &procinfo)) throw Win32Exception();

		DeleteProcThreadAttributeList(attributes);			// Clean up the PROC_THREAD_ATTRIBUTE_LIST
	}

	catch(...) { DeleteProcThreadAttributeList(attributes); throw; }

	// Process was successfully created and initialized, pass it off to a Host instance
	return std::make_unique<Host>(procinfo);
}

//-----------------------------------------------------------------------------
// Host::Resume
//
// Resumes a suspended host process
//
// Arguments:
//
//	NONE

void Host::Resume(void)
{
	NTSTATUS result = NtResumeProcess(m_procinfo.hProcess);
	if(result != 0) throw StructuredException(result);
}

//-----------------------------------------------------------------------------
// Host::Suspend
//
// Suspends the hosted process
//
// Arguments:
//
//	NONE

void Host::Suspend(void)
{
	NTSTATUS result = NtSuspendProcess(m_procinfo.hProcess);
	if(result != 0) throw StructuredException(result);
}

//-----------------------------------------------------------------------------
// Host::Terminate
//
// Terminates the hosted process
//
// Arguments:
//
//	exitcode	- Exit code to use for process termination

void Host::Terminate(HRESULT exitcode)
{
	if(!TerminateProcess(m_procinfo.hProcess, static_cast<UINT>(exitcode))) throw Win32Exception();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
