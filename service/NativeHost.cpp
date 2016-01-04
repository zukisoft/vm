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
#include "NativeHost.h"

#include "LinuxException.h"
#include "NativeProcess.h"
#include "NativeThread.h"
#include "SystemInformation.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// NativeHost::Create
//
// Creates a new native operating system host process/thread pair
//
// Arguments:
//
//	path			- Path to the native process executable
//	arguments		- Arguments to pass to the executable

std::tuple<std::unique_ptr<NativeProcess>, std::unique_ptr<NativeThread>> NativeHost::Create(const tchar_t* path, const tchar_t* arguments)
{
	return Create(path, arguments, nullptr, 0);
}

//-----------------------------------------------------------------------------
// NativeHost::Create
//
// Creates a new native operating system host process/thread pair
//
// Arguments:
//
//	path			- Path to the native process executable
//	arguments		- Arguments to pass to the executable
//	handles			- Optional array of inheritable handle objects
//	numhandles		- Number of elements in the handles array

std::tuple<std::unique_ptr<NativeProcess>, std::unique_ptr<NativeThread>> NativeHost::Create(const tchar_t* path, const tchar_t* arguments, HANDLE handles[], size_t numhandles)
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
	if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) throw LinuxException{ LINUX_EACCES, Win32Exception{ GetLastError() } };

	// Allocate a buffer large enough to hold the attribute data and initialize it
	auto buffer = std::make_unique<uint8_t[]>(required);
	PPROC_THREAD_ATTRIBUTE_LIST attributes = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(&buffer[0]);
	if(!InitializeProcThreadAttributeList(attributes, 1, 0, &required)) throw LinuxException{ LINUX_EACCES, Win32Exception{ GetLastError() } };

	try {

		// UpdateProcThreadAttribute will fail if there are no handles in the specified array
		if((handles != nullptr) && (numhandles > 0)) {
			
			// Add the array of handles as inheritable handles for the client process
			if(!UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles, numhandles * sizeof(HANDLE),
				nullptr, nullptr)) throw LinuxException{ LINUX_EACCES, Win32Exception{ GetLastError() } };
		}

		// Attempt to launch the process using the CREATE_SUSPENDED and EXTENDED_STARTUP_INFO_PRESENT flag
		zero_init<STARTUPINFOEX> startinfo;
		startinfo.StartupInfo.cb = sizeof(STARTUPINFOEX);
		startinfo.lpAttributeList = attributes;
		if(!CreateProcess(path, commandline, nullptr, nullptr, TRUE, CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT, nullptr, 
			nullptr, &startinfo.StartupInfo, &procinfo)) throw LinuxException{ LINUX_EACCES, Win32Exception{ GetLastError() } };

		DeleteProcThreadAttributeList(attributes);			// Clean up the PROC_THREAD_ATTRIBUTE_LIST
	}

	catch(...) { DeleteProcThreadAttributeList(attributes); throw; }

	// Get the actual architecture of the created process/thread, don't rely on what was requested
	enum class Architecture arch = GetProcessArchitecture(procinfo.hProcess);

	// Transfer ownership of the process and main thread handles to NativeProcess/NativeThread
	// todo: there should be exception handling here to kill process and close handles
	return std::make_tuple(std::make_unique<NativeProcess>(arch, procinfo.hProcess, procinfo.dwProcessId), 
		std::make_unique<NativeThread>(arch, procinfo.hThread, procinfo.dwThreadId));
}

//-----------------------------------------------------------------------------
// NativeHost::GetProcessArchitecture (private, static)
//
// Determines the Architecture of a native process
//
// Arguments:
//
//	process		- Native process handle

enum class Architecture NativeHost::GetProcessArchitecture(HANDLE process)
{
	BOOL				result;				// Result from IsWow64Process

	// If the operating system is 32-bit, the architecture must be x86
	if(SystemInformation::ProcessorArchitecture == SystemInformation::Architecture::Intel) return Architecture::x86;

	// 64-bit operating system, check the WOW64 status of the process to determine architecture
	if(!IsWow64Process(process, &result)) throw Win32Exception{};

	return (result) ? Architecture::x86 : Architecture::x86_64;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
