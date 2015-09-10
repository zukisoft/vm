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
#include "NativeProcess.h"

#include "SystemInformation.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// NativeProcess Constructor
//
// Arguments:
//
//	architecture	- Native process architecture flag
//	procinfo		- Process information structure

NativeProcess::NativeProcess(enum class Architecture architecture, PROCESS_INFORMATION& procinfo) :
	m_architecture(architecture), m_process(procinfo.hProcess), m_processid(procinfo.dwProcessId), m_thread(procinfo.hThread), m_threadid(procinfo.dwThreadId) 
{
}

//-----------------------------------------------------------------------------
// NativeProcess Destructor

NativeProcess::~NativeProcess()
{
	CloseHandle(m_thread);
	CloseHandle(m_process);
}

//-----------------------------------------------------------------------------
// NativeProcess::getArchitecture
//
// Gets the architecture of the native process

enum class Architecture NativeProcess::getArchitecture(void) const
{
	return m_architecture;
}
	
//-----------------------------------------------------------------------------
// NativeProcess::Create
//
// Creates a new native operating system process instance
//
// Arguments:
//
//	path			- Path to the native process executable
//	arguments		- Arguments to pass to the executable

std::unique_ptr<NativeProcess> NativeProcess::Create(const tchar_t* path, const tchar_t* arguments)
{
	return Create(path, arguments, nullptr, 0);
}

//-----------------------------------------------------------------------------
// NativeProcess::Create
//
// Creates a new native operating system process instance
//
// Arguments:
//
//	path			- Path to the native process executable
//	arguments		- Arguments to pass to the executable
//	handles			- Optional array of inheritable handle objects
//	numhandles		- Number of elements in the handles array

std::unique_ptr<NativeProcess> NativeProcess::Create(const tchar_t* path, const tchar_t* arguments, HANDLE handles[], size_t numhandles)
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
	auto buffer = std::make_unique<uint8_t[]>(required);
	PPROC_THREAD_ATTRIBUTE_LIST attributes = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(&buffer[0]);
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

	// Process was successfully created and initialized, construct the NativeProcess instance
	try { return std::make_unique<NativeProcess>(GetProcessArchitecture(procinfo.hProcess), procinfo); }

	catch(...) {

		// It's unlikely that the creation of NativeProcess would fail, but if it does clean up
		TerminateProcess(procinfo.hProcess, ERROR_PROCESS_ABORTED);
		CloseHandle(procinfo.hThread);
		CloseHandle(procinfo.hProcess);
		throw;
	}
}

//-----------------------------------------------------------------------------
// NativeProcess::getExitCode
//
// Gets the exit code of the process, or STILL_ACTIVE if it's still running

DWORD NativeProcess::getExitCode(void) const
{
	DWORD				result;				// Result from GetExitCodeProcess

	if(!GetExitCodeProcess(m_process, &result)) throw Win32Exception{};

	return result;
}

//-----------------------------------------------------------------------------
// NativeProcess::GetProcessArchitecture (private, static)
//
// Determines the Architecture of a native process
//
// Arguments:
//
//	process		- Native process handle

enum class Architecture NativeProcess::GetProcessArchitecture(HANDLE process)
{
	BOOL				result;				// Result from IsWow64Process

	// If the operating system is 32-bit, the architecture must be x86
	if(SystemInformation::ProcessorArchitecture == SystemInformation::Architecture::Intel) return Architecture::x86;

	// 64-bit operating system, check the WOW64 status of the process to determine architecture
	if(!IsWow64Process(process, &result)) throw Win32Exception{};

	return (result) ? Architecture::x86 : Architecture::x86_64;
}

//-----------------------------------------------------------------------------
// NativeProcess::getProcessHandle
//
// Gets the host process handle

HANDLE NativeProcess::getProcessHandle(void) const
{
	return m_process;
}

//-----------------------------------------------------------------------------
// NativeProcess::getProcessId
//
// Gets the host process identifier

DWORD NativeProcess::getProcessId(void) const
{
	return m_processid;
}

//-----------------------------------------------------------------------------
// NativeProcess::Terminate
//
// Terminates the native process;
//
// Arguments:
//
//	exitcode		- Exit code for the process

void NativeProcess::Terminate(uint16_t exitcode) const
{
	Terminate(exitcode, true);		// Wait for the process to terminate
}

//-----------------------------------------------------------------------------
// NativeProcess::Terminate
//
// Terminates the native process
//
// Arguments:
//
//	exitcode		- Exit code for the process
//	wait			- Flag to wait for the process to exit

void NativeProcess::Terminate(uint16_t exitcode, bool wait) const
{
	TerminateProcess(m_process, static_cast<UINT>(exitcode));
	if(wait) WaitForSingleObject(m_process, INFINITE);
}

//-----------------------------------------------------------------------------
// NativeProcess::getThreadHandle
//
// Gets the host main thread handle

HANDLE NativeProcess::getThreadHandle(void) const
{
	return m_thread;
}

//-----------------------------------------------------------------------------
// NativeProcess::getThreadId
//
// Gets the host main thread identifier

DWORD NativeProcess::getThreadId(void) const
{
	return m_threadid;
}
	
//-----------------------------------------------------------------------------

#pragma warning(pop)
