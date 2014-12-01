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
#include "resource.h"
#include "CommandLine.h"
#include "Console.h"
#include "StructuredException.h"
#include "VmService.h"

#pragma warning(push, 4)

//---------------------------------------------------------------------------
// _tWinMain
//
// Main application entry point
//
// Arguments :
//
//	instance		- Application instance handle
//	previnstance	- Unused in Win32, always set to NULL
//	cmdline			- Pointer to application command line string
//	show			- Application initial display flags

int APIENTRY _tWinMain(HINSTANCE, HINSTANCE, LPTSTR cmdline, int)
{
#ifdef _DEBUG

	int nDbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);	// Get current flags
	nDbgFlags |= _CRTDBG_LEAK_CHECK_DF;						// Enable leak-check
	_CrtSetDbgFlag(nDbgFlags);								// Set the new flags/

#endif	// _DEBUG

	// Initialize the SEH to C++ exception translator
	_set_se_translator(StructuredException::SeTranslator);

	// Convert the provided command line into a CommandLine instance
	CommandLine commandline(cmdline);

	// Register the RPC protocol sequences that will be used by all the services
	// hosted within this process
	// TODO: Security Descriptor
	RPC_STATUS rpcresult = RpcServerUseProtseq((RPC_WSTR)L"ncalrpc", RPC_C_PROTSEQ_MAX_REQS_DEFAULT, nullptr);
	if(rpcresult != RPC_S_OK) {
		throw std::exception("todo: bad thing");
	}

	// -console
	//
	// Run the service as a standalone console application rather than a service
	if(commandline.Switches.Contains(L"console")) {

		Console console(L"VM Service Console");

		// todo: make sure -initramfs: switch and value exists
		ServiceHarness<VmService> harness;
//#ifdef _M_X64
//		harness.SetParameter(_T("vm.initramfs"), _T("D:\\rootfs_x64.cpio.gz")); //commandline.Switches.GetValue(L"initramfs"));
//#else
		harness.SetParameter(_T("vm.initramfs"), _T("D:\\rootfs_x86.cpio.gz"));
//#endif

		// test parameters
		harness.SetParameter(_T("systemlog.length"), 1 MiB);
#ifdef _DEBUG
		harness.SetParameter(_T("process.host.32bit"), _T("D:\\GitHub\\vm\\out\\Win32\\Debug\\zuki.vm.host32.exe"));
		harness.SetParameter(_T("process.host.64bit"), _T("D:\\GitHub\\vm\\out\\x64\\Debug\\zuki.vm.host64.exe"));
#else
		harness.SetParameter(_T("process.host.32bit"), _T("D:\\GitHub\\vm\\out\\Win32\\Release\\zuki.vm.host32.exe"));
		harness.SetParameter(_T("process.host.64bit"), _T("D:\\GitHub\\vm\\out\\x64\\Release\\zuki.vm.host64.exe"));
#endif
		harness.SetParameter(_T("process.host.timeout"), 10000);
		//harness.SetParameter(_T("vm.initpath"), _T("/sbin/init"));
		harness.SetParameter(_T("vm.initpath"), _T("/init"));

		harness.Start(IDS_VMSERVICE_NAME);
		//harness.WaitForStatus(ServiceStatus::Running);  <--- done automatically by Start()

		// 128/129 == TEST HOST PROCESS CREATION
		harness.SendControl((ServiceControl)128);
		harness.SendControl((ServiceControl)129);

		console.WriteLine(L"VM SERVICE RUNNING");
		console.WriteLine();
		console.WriteLine(L"Press ENTER to exit");
		console.ReadLine();

		if(harness.CanStop) harness.Stop();
	}

	// -service
	//
	// Run the service normally, using the specified short name for the VmService instance
	else if(commandline.Switches.Contains(L"service")) {

		// todo: make sure the service name has been specified; this may change to a UUID for RPC binding instead
		ServiceTable services = { ServiceTableEntry<VmService>(commandline.Switches.GetValue(L"service")) };
		services.Dispatch();
	}

	else {

		// TODO: invalid command line; requires -console or -service
	}

	return 0;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
