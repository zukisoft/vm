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

//-----------------------------------------------------------------------------

Host::Host(const PROCESS_INFORMATION& procinfo)
{
	m_procinfo = procinfo;
}

Host::~Host()
{
	// Close the process handles created along with the host process
	CloseHandle(m_procinfo.hThread);
	CloseHandle(m_procinfo.hProcess);
}

std::unique_ptr<Host> Host::Create(const tchar_t* binarypath, const tchar_t* commandline)
{
	zero_init<PROCESS_INFORMATION>	procinfo;			// Process information
	//zero_init<STARTUPINFOEX>		startinfo;			// Startup information
	zero_init<STARTUPINFO>			startinfo;			// Startup information
	HANDLE							readysignal;		// Event host signals when ready

	// Initialize the STARTUPINFO structure for the new process
	startinfo.cb = sizeof(STARTUPINFO); // <--- CHANGE TO EX

	zero_init<SECURITY_ATTRIBUTES> secattr;
	secattr.nLength = sizeof(SECURITY_ATTRIBUTES);
	secattr.bInheritHandle = TRUE;
	readysignal = CreateEvent(&secattr, TRUE, FALSE, nullptr);
	if(!readysignal) throw Win32Exception();

	size_t required;
	if(!InitializeProcThreadAttributeList(nullptr, 1, 0, &required)) throw Win32Exception();
	std::vector<uint8_t> buffer(required);
	//if(!InitializeProcThreadAttributeList(reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.data()), 1, 0, &required)) throw Win32Exception();
	//if(!UpdateProcThreadAttribute(reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.data()), 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, 

	// Create a copy of the command line string, it may be modified by CreateProcess()
	tchar_t* cmdline = _tcsdup(commandline);

	BOOL result = CreateProcess(binarypath, cmdline, nullptr, nullptr, TRUE, CREATE_SUSPENDED /*| EXTENDED_STARTUPINFO_PRESENT*/, 
		nullptr, nullptr, &startinfo, &procinfo);

	free(cmdline);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
