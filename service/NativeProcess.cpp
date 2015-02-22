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

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// NativeProcess Constructor
//
// Arguments:
//
//	process			- Process handle instance
//	processid		- Native process identifier
//	thread			- Thread handle instance
//	threadid		- Native thread identifier

NativeProcess::NativeProcess(std::shared_ptr<NativeHandle>&& process, DWORD processid, std::shared_ptr<NativeHandle>&& thread, DWORD threadid) :
	m_process(std::move(process)), m_processid(processid), m_thread(std::move(thread)), m_threadid(threadid) {}

//-----------------------------------------------------------------------------
// NativeProcess::CheckArchitecture<x86> (private, static)
//
// Checks that the created host process is 32-bit
//
// Arguments:
//
//	instance		- NativeProcess instance to be validated

template<> 
void NativeProcess::CheckArchitecture<Architecture::x86>(const std::unique_ptr<NativeProcess>& instance)
{
	BOOL			result;				// Result from IsWow64Process

	// 32-bit systems can only create 32-bit processes; nothing to worry about
	if(SystemInformation::ProcessorArchitecture == SystemInformation::Architecture::Intel) return;

	// 64-bit system; verify that the process is running under WOW64
	if(!IsWow64Process(instance->Process->Handle, &result)) throw Win32Exception();
	if(!result) throw Exception(E_PROCESSINVALIDX86HOST);		// todo: rename exception
}

//-----------------------------------------------------------------------------
// Process::CheckArchitecture<x86_64> (static, private)
//
// Checks that the created host process is 64-bit
//
// Arguments:
//
//	instance		- NativeProcess instance to be validated

#ifdef _M_X64
template<>
void NativeProcess::CheckArchitecture<Architecture::x86_64>(const std::unique_ptr<NativeProcess>& instance)
{
	BOOL				result;				// Result from IsWow64Process

	// 64-bit system; verify that the process is not running under WOW64
	if(!IsWow64Process(instance->Process->Handle, &result)) throw Win32Exception();
	if(result) throw Exception(E_PROCESSINVALIDX64HOST);		// todo: rename exception
}
#endif

//-----------------------------------------------------------------------------
// NativeProcess::Create<Architecture::x86>
//
// Arguments:
//
//	vm			- Parent virtual machine instance

template<>
std::unique_ptr<NativeProcess> NativeProcess::Create<Architecture::x86>(const std::shared_ptr<VirtualMachine>& vm)
{
	// Get the architecture-specific filename and arguments from the virtual machine instance
	std::tstring filename = vm->GetProperty(VirtualMachine::Properties::HostProcessBinary32);
	std::tstring arguments = vm->GetProperty(VirtualMachine::Properties::HostProcessArguments);

	// Construct the specified process and validate that it's Architecture::x86
	std::unique_ptr<NativeProcess> host = Create(filename.c_str(), arguments.c_str(), nullptr, 0);
	CheckArchitecture<Architecture::x86>(host);

	return host;
}

//-----------------------------------------------------------------------------
// NativeProcess::Create<Architecture::x86_64>
//
// Arguments:
//
//	vm			- Parent virtual machine instance

#ifdef _M_X64
template<>
std::unique_ptr<NativeProcess> NativeProcess::Create<Architecture::x86>(const std::shared_ptr<VirtualMachine>& vm)
{
	// Get the architecture-specific filename and arguments from the virtual machine instance
	std::tstring filename = vm->GetProperty(VirtualMachine::Properties::HostProcessBinary64);
	std::tstring arguments = vm->GetProperty(VirtualMachine::Properties::HostProcessArguments);

	// Construct the specified process and validate that it's Architecture::x86
	std::unique_ptr<NativeProcess> host = Create(filename.c_str(), arguments.c_str(), nullptr, 0);
	CheckArchitecture<Architecture::x86_64>(host);

	return host;
}
#endif

//-----------------------------------------------------------------------------
// NativeProcess::Create (private, static)
//
// Creates a new native operating system process instance
//
// Arguments:
//
//	path			- Path to the host binary
//	arguments		- Arguments to pass to the host binary
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

	// Process was successfully created and initialized, construct the NativeProcess instance
	return std::make_unique<NativeProcess>(NativeHandle::FromHandle(procinfo.hProcess), procinfo.dwProcessId,
		NativeHandle::FromHandle(procinfo.hThread), procinfo.dwThreadId);
}

//-----------------------------------------------------------------------------
// NativeProcess::Process
//
// Gets the host process handle

std::shared_ptr<NativeHandle> NativeProcess::getProcess(void) const
{
	return m_process;
}

//-----------------------------------------------------------------------------
// NativeProcess::ProcessId
//
// Gets the host process identifier

DWORD NativeProcess::getProcessId(void) const
{
	return m_processid;
}

//-----------------------------------------------------------------------------
// NativeProcess::Thread
//
// Gets the host main thread handle

std::shared_ptr<NativeHandle> NativeProcess::getThread(void) const
{
	return m_thread;
}

//-----------------------------------------------------------------------------
// NativeProcess::ThreadId
//
// Gets the host main thread identifier

DWORD NativeProcess::getThreadId(void) const
{
	return m_threadid;
}
	
//-----------------------------------------------------------------------------

#pragma warning(pop)
