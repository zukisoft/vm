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
#include "VmService.h"

#pragma warning(push, 4)

//---------------------------------------------------------------------------
// VmService::OnStart (private)
//
// Invoked when the service is started
//
// Arguments :
//
//	argc		- Number of command line arguments
//	argv		- Array of command line argument strings

void VmService::OnStart(int, LPTSTR*)
{
	LARGE_INTEGER				qpcbias;			// QueryPerformanceCounter bias

	// The system log needs to know what value acts as zero for the timestamps
	QueryPerformanceCounter(&qpcbias);

	// Create the system log instance and seed the time bias to now
	m_syslog = std::make_unique<VmSystemLog>(m_sysloglength);
	m_syslog->TimestampBias = qpcbias.QuadPart;

	// TODO: Put a real log here with the zero-time bias and the size of the
	// configured system log
	m_syslog->Push("System log initialized");
	////

	// INITIALIZE RAMDISK
	svctl::tstring initramfs = m_initramfs;
	// Attempt to load the initial ramdisk file system
	// this needs unwind on exception?
	// m_vfs.LoadInitialFileSystem(initramfs.c_str());

	// REGISTER RPC INTERFACES
	try {

		// TODO: I want to rework the RpcInterface thing at some point, but this
		// is a LOT cleaner than making the RPC calls here.  Should also have
		// some kind of rundown to deal with exceptions properly

		// This may work better as a general single registrar in syscalls/SystemCalls
		// since the service really doesn't care that it has 2 RPC interfaces, they
		// both just come back to this single service instance via the entry point vectors
		syscall32_listener::Register(RPC_IF_AUTOLISTEN);
		syscall32_listener::AddObject(this->ObjectID32); 
		m_bindstr32 = syscall32_listener::GetBindingString(this->ObjectID32);
		// m_syslog->Push

#ifdef _M_X64
		syscall64_listener::Register(RPC_IF_AUTOLISTEN);
		syscall64_listener::AddObject(this->ObjectID64);
		m_bindstr64 = syscall64_listener::GetBindingString(this->ObjectID64);
		// m_syslog->Push
#endif
	} 

	// RpcInterface throws Win32Exception, servicelib expects ServiceException
	catch(Win32Exception& ex) { throw ServiceException(ex.HResult); }

	// INITIALIZE PROCESS MANAGER
	//

	// TODO: throwing an exception here messes up the ServiceHarness, you get
	// that damn "abort() has been called" from the std::thread ... fix that 
	// in servicelib at some point in the near future
}

//-----------------------------------------------------------------------------
// VmService::OnStop (private)
//
// Invoked when the service is stopped
//
// Arguments:
//
//	NONE

void VmService::OnStop(void)
{

#ifdef _M_X64
	syscall64_listener::RemoveObject(this->ObjectID64);
	syscall64_listener::Unregister(true);
#endif

	syscall32_listener::RemoveObject(this->ObjectID32);
	syscall32_listener::Unregister(true);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
