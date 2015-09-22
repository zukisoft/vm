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
#include "resource.h"
#include "CommandLine.h"
#include "Console.h"
#include "StructuredException.h"
#include "VirtualMachine.h"

#include "Host2.h"

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
	_CrtSetDbgFlag(nDbgFlags);								// Set the new flags

#endif	// _DEBUG

	/////////////////
	Host2 host(GetCurrentProcess());

	for(int index = 0; index < 100; index++) VirtualAlloc(nullptr, 65536, MEM_RESERVE, PAGE_NOACCESS);

	uintptr_t test = host.Allocate(0x08000000, 65536, (VirtualMemory::Protection::Write |VirtualMemory::Protection::Guard));		// 64K  --> 0x08010000
	uintptr_t test3 = host.Allocate(0x08010000, 65536, VirtualMemory::Protection::Write);
	uintptr_t test2 = host.Allocate(0x08020000, 65536, VirtualMemory::Protection::Read);	// 64K  --> 0x08030000

	void* local = host.Map(0x08000000, 0x100, (VirtualMemory::Protection::Execute | VirtualMemory::Protection::Read | VirtualMemory::Protection::Write));

	auto buffer = std::make_unique<uint8_t[]>(256 KiB);

	host.Read(0x08002000, &buffer[0], 80 KiB);
	host.Write(0x08002000, &buffer[0], 64 KiB);

	host.Release(0x08010000, 32768);
	host.Release(0x08010000, 65536);
	host.Protect(0x08000000, 8192, VirtualMemory::Protection::Execute);
	host.Release(0x08000000, 65536);

	*reinterpret_cast<uint32_t*>(local) = 0x12345678;
	//host.Unmap(local);

	//host.Lock(0x08000000, 30000);

	//host.Allocate(test + 1048576, 120000, (VirtualMemory::Protection::Read | VirtualMemory::Protection::Execute));
	return 0;

	////////////////

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
		//ServiceHarness<VmService> harness;
		ServiceHarness<VirtualMachine> harness;
////#ifdef _M_X64
////		harness.SetParameter(_T("vm.initramfs"), _T("D:\\rootfs_x64.cpio.gz")); //commandline.Switches.GetValue(L"initramfs"));
////#else
//		harness.SetParameter(_T("vm.initramfs"), _T("D:\\rootfs_x86.cpio.gz"));
////#endif

		// test parameters
		harness.SetParameter(_T("systemlog.length"), 1 MiB);
#ifdef _DEBUG
		harness.SetParameter(_T("vm.host32"), _T("D:\\GitHub\\vm\\out\\Win32\\Debug\\zuki.vm.host32.exe"));
		harness.SetParameter(_T("vm.host64"), _T("D:\\GitHub\\vm\\out\\x64\\Debug\\zuki.vm.host64.exe"));
#else
		harness.SetParameter(_T("process.host.32bit"), _T("D:\\GitHub\\vm\\out\\Win32\\Release\\zuki.vm.host32.exe"));
		harness.SetParameter(_T("process.host.64bit"), _T("D:\\GitHub\\vm\\out\\x64\\Release\\zuki.vm.host64.exe"));
#endif
		harness.SetParameter(_T("process.host.timeout"), 10000);
		//harness.SetParameter(_T("vm.initpath"), _T("/sbin/init"));
		//harness.SetParameter(_T("vm.initpath"), _T("/init"));


		// new parameters
		//harness.SetParameter(_T("init"), _T("/init"));
		harness.SetParameter(_T("init"), _T("/system/bin/rild"));		// <--- testing elf image
		//harness.SetParameter(_T("initrd"), _T("D:\\rootfs_x86.cpio.gz"));
		harness.SetParameter(_T("rootfstype"), _T("hostfs"));
		//harness.SetParameter(_T("root"), _T("D:\\Linux Stuff\\android-5.0.2_r1-x86\\root"));
		harness.SetParameter(_T("root"), _T("D:\\Linux Stuff\\android-5.0.2_r1-x86\\"));
		harness.SetParameter(_T("rootflags"), _T("ro,sandbox"));

		harness.Start(IDS_VMSERVICE_NAME);
		//harness.WaitForStatus(ServiceStatus::Running);  <--- done automatically by Start()

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
		//ServiceTable services = { ServiceTableEntry<VmService>(commandline.Switches.GetValue(L"service")) };
		ServiceTable services = { ServiceTableEntry<VirtualMachine>(commandline.Switches.GetValue(L"service")) };
		services.Dispatch();
	}

	else {

		// TODO: invalid command line; requires -console or -service
	}

	return 0;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
