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
#include <Exception.h>
#include "resource.h"
#include "ElfImage.h"
#include "SystemCall.h"

LONG CALLBACK SysCallExceptionHandler(PEXCEPTION_POINTERS exception);
HMODULE GetMyModuleTest(void);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	ElfImage*		executable = nullptr;				// Executable image
	ElfImage*		interpreter = nullptr;				// Interpreter image


	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//const tchar_t* exec_path = _T("D:\\Linux Binaries\\generic_x86\\system\\bin\\bootanimation");
	const tchar_t* exec_path = _T("D:\\android\\init");
	//const tchar_t* exec_path = _T("D:\\Linux Binaries\\generic_x86\\system\\bin\\linker");
	//const tchar_t* exec_path = _T("D:\\Linux Binaries\\busybox-x86");
	//const tchar_t* exec_path = _T("D:\\Linux Binaries\\bionicapp");
	//const tchar_t* exec_path = _T("D:\\Linux Binaries\\generic_x86\\root\\init");


	// VDSO - need to work on this, not to mention I need an x86 and an x64 version of it
	ElfImage* vdso = ElfImage::FromResource(MAKEINTRESOURCE(IDR_RCDATA_VDSO32INT80), RT_RCDATA);

	ElfArguments builder;

	// Clone the command line arguments into the auxiliary vector; these are expected
	// to be correct for the hosted process, including argument zero, on entry
	builder.AppendArgument(exec_path);
	for(int index = 1; index < __argc; index++) builder.AppendArgument(__targv[index]);

	// environment should come from the remote services

	// Clone the initial environment into the auxiliary vector
	tchar_t** env = _tenviron;
	while(*env) { builder.AppendEnvironmentVariable(*env); env++; }

	// AT_RANDOM
	GUID pseudorandom;
	CoCreateGuid(&pseudorandom);

	HMODULE mod = GetMyModuleTest();

	try { 
		
		// note: would use a while loop to iterate over interpreters, they could be chained
		executable = ElfImage::FromFile(exec_path);
		if(executable->Interpreter) {

			// TEST: sys005_open()
			int test = SystemCall(mod, 5).Invoke(executable->Interpreter, 0, 0);

			interpreter = ElfImage::FromFile(_T("D:\\Linux Binaries\\generic_x86\\system\\bin\\linker"));
			if(interpreter->Interpreter) {

				// TODO: throw something here; don't support chained interpreters yet
			}
		}

		//
		// AUXILIARY VECTORS
		//

		(LINUX_AT_EXECFD);		// 2 - - CAN IMPLEMENT ONCE FSMANAGER IS WORKING
		
		if(executable->ProgramHeaders) {

			builder.AppendAuxiliaryVector(LINUX_AT_PHDR, executable->ProgramHeaders);				// 3
#ifdef _M_X64
			builder.AppendAuxiliaryVector(LINUX_AT_PHENT, sizeof(uapi::Elf64_Phdr));					// 4
#else
			builder.AppendAuxiliaryVector(LINUX_AT_PHENT, sizeof(uapi::Elf32_Phdr));					// 4
#endif
			builder.AppendAuxiliaryVector(LINUX_AT_PHNUM, executable->NumProgramHeaders);			// 5
		}

		builder.AppendAuxiliaryVector(LINUX_AT_PAGESZ, MemoryRegion::PageSize);					// 6

		// AT_BASE is only used with an interpreter and specifies that module's base address
		if(interpreter) builder.AppendAuxiliaryVector(LINUX_AT_BASE, interpreter->BaseAddress);	// 7

		builder.AppendAuxiliaryVector(LINUX_AT_FLAGS, 0);									// 8
		builder.AppendAuxiliaryVector(LINUX_AT_ENTRY, executable->EntryPoint);			// 9
		(LINUX_AT_NOTELF);		// 10 - DO NOT IMPLEMENT
		(LINUX_AT_UID);			// 11
		(LINUX_AT_EUID);			// 12
		(LINUX_AT_GID);			// 13
		(LINUX_AT_EGID);			// 14
#ifdef _M_X64
		builder.AppendAuxiliaryVector(LINUX_AT_PLATFORM, "x86_64");						// 15
#else
		builder.AppendAuxiliaryVector(LINUX_AT_PLATFORM, "i686");							// 15
#endif
		(LINUX_AT_HWCAP);			// 16
		(LINUX_AT_CLKTCK);		// 17
		builder.AppendAuxiliaryVector(LINUX_AT_SECURE, 0);								// 23
		(LINUX_AT_BASE_PLATFORM);	// 24 - DO NOT IMPLEMENT
		builder.AppendAuxiliaryVector(LINUX_AT_RANDOM, &pseudorandom, sizeof(GUID));		// 25
		(LINUX_AT_HWCAP2);		// 26 - DO NOT IMPLEMENT
		(LINUX_AT_EXECFN);		// 31
		(LINUX_AT_SYSINFO);		// 32 - PROBABLY DO NOT IMPLEMENT
		builder.AppendAuxiliaryVector(LINUX_AT_SYSINFO_EHDR, vdso->BaseAddress);			// 33

		// add exception handler from the system calls dll
		AddVectoredExceptionHandler(1, SysCallExceptionHandler);

		if(interpreter) interpreter->Execute(builder);
		else executable->Execute(builder);

		if(interpreter) delete interpreter;
		delete executable;
	}
	catch(Exception& ex) {
		MessageBox(NULL, ex.Message, _T("Exception"), MB_OK | MB_ICONHAND);
		return (int)E_FAIL;
	}

	//ElfArguments::ReleaseArgumentStack(args);

	return 0;
}
