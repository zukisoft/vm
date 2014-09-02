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

#include "RootFileSystem.h"
#include "VmFileSystem.h"

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

	// Initialize the SEH to C++ exception translator
	_set_se_translator(StructuredException::SeTranslator);

	//
	// TEMPFS TESTING
	//

	std::unique_ptr<VmFileSystem> vfs = VmFileSystem::Create(RootFileSystem::Mount(nullptr));
	vfs->Mount(nullptr, L"/", L"tmpfs", 0, nullptr);

	try {

		FileSystem::HandlePtr p = vfs->CreateFile(L"test1", LINUX_O_RDWR, 0);
		//vfs->CreateDirectory(L"test1/test2");
		//vfs->CreateDirectory(L"test1/test2/test3");
		//vfs->CreateSymbolicLink(L"test1/test2/test3/mysymlink", L"../../test2");
		//vfs->CreateDirectory(L"test1/test2/test3/mysymlink/test4");
		//vfs->CreateDirectory(L"test1/test2/test4/test5");

		//vfs->Open(L"/mike", 0);

		std::vector<uint8_t> buffer(2048, 0xAA);
		std::vector<uint8_t> buffer2(2048);

		auto count = p->Write(buffer.data(), 2048);
		count = p->Read(buffer2.data(), 2048);
		int x = 123;
	}
	catch(const std::exception& ex) {
	
		int x = 123;
	}

	return 0;	
	
	//
	// END TESTING
	//

	// Convert the provided command line into a CommandLine instance
	CommandLine commandline(cmdline);

	////////
	RPC_STATUS rpcresult = RpcServerUseAllProtseqsIf(RPC_C_PROTSEQ_MAX_REQS_DEFAULT, SystemCalls_v1_0_s_ifspec, nullptr);
	if(rpcresult != RPC_S_OK) {
	}
	///////

	// -console
	//
	// Run the service as a standalone console application rather than a service
	if(commandline.Switches.Contains(L"console")) {

		Console console(L"VM Service Console");

		// todo: make sure -initramfs: switch and value exists
		ServiceHarness<VmService> harness;
		harness.SetParameter(IDR_PARAM_INITRAMFS, commandline.Switches.GetValue(L"initramfs"));

		harness.SetParameter(IDR_PARAM_SYSLOGLENGTH, 1 MiB);

		harness.Start(IDS_VMSERVICE_NAME);

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
