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
#include "Exception.h"
#include "ElfImage.h"
#include "SystemCall.h"

LONG CALLBACK SysCallExceptionHandler(PEXCEPTION_POINTERS exception);
HMODULE GetMyModuleTest(void);

// UnhandledException
//
// 
LONG CALLBACK UnhandledException(PEXCEPTION_POINTERS exception)
{
	// Get the current process and thread handles
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();

	// Make a copy of the context so it doesn't get trashed
	CONTEXT context;
	memcpy(&context, exception->ContextRecord, sizeof(CONTEXT));
	
	// Allocate and initialize the STACKFRAME64 structure for the walk
	STACKFRAME64 stackframe;
	memset(&stackframe, sizeof(STACKFRAME64), 0);
	stackframe.AddrPC.Mode = AddrModeFlat;
	stackframe.AddrPC.Offset = context.Eip;
	stackframe.AddrStack.Mode = AddrModeFlat;
	stackframe.AddrStack.Offset = context.Esp;
	stackframe.AddrFrame.Mode = AddrModeFlat;
	stackframe.AddrFrame.Offset = context.Ebp;

	// Allocate the symbol lookup structure, which includes space for the name
	uint8_t symbuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(CHAR)];
	PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(symbuffer);
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbol->MaxNameLen = MAX_SYM_NAME;

	SymFromAddr(process, context.Eip, 0, symbol);
	OutputDebugString(_T("unhandled exception in function: "));
	OutputDebugStringA(symbol->Name);
	OutputDebugString(_T("\r\nstrack trace:\r\n"));

	// Walk the stack until it cannot be walked any further ...
	while(StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &stackframe, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {

		SymFromAddr(process, stackframe.AddrPC.Offset, 0, symbol);
		OutputDebugStringA(symbol->Name);
		OutputDebugString(_T("\r\n"));
	} ;

	return EXCEPTION_CONTINUE_SEARCH;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	ElfImage*		executable = nullptr;				// Executable image
	ElfImage*		interpreter = nullptr;				// Interpreter image


	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	const tchar_t* exec_path = _T("D:\\Linux Binaries\\generic_x86\\system\\bin\\bootanimation");
	//const tchar_t* exec_path = _T("D:\\test");
	//const tchar_t* exec_path = _T("D:\\Linux Binaries\\generic_x86\\system\\bin\\linker");
	//const tchar_t* exec_path = _T("D:\\Linux Binaries\\busybox-x86");
	//const tchar_t* exec_path = _T("D:\\Linux Binaries\\bionicapp");
	//const tchar_t* exec_path = _T("D:\\Linux Binaries\\generic_x86\\root\\init");


	AddVectoredExceptionHandler(0, UnhandledException);

	BOOL bresult = SymInitialize(GetCurrentProcess(), NULL, FALSE);

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

		(AT_EXECFD);		// 2 - DO NOT IMPLEMENT
		
		if(executable->ProgramHeaders) {

			builder.AppendAuxiliaryVector(AT_PHDR, executable->ProgramHeaders);				// 3
#ifdef _M_X64
			builder.AppendAuxiliaryVector(AT_PHENT, sizeof(Elf64_Phdr));					// 4
#else
			builder.AppendAuxiliaryVector(AT_PHENT, sizeof(Elf32_Phdr));					// 4
#endif
			builder.AppendAuxiliaryVector(AT_PHNUM, executable->NumProgramHeaders);			// 5
		}

		builder.AppendAuxiliaryVector(AT_PAGESZ, MemoryRegion::PageSize);					// 6

		// AT_BASE is only used with an interpreter and specifies that module's base address
		if(interpreter) builder.AppendAuxiliaryVector(AT_BASE, interpreter->BaseAddress);	// 7

		builder.AppendAuxiliaryVector(AT_FLAGS, 0);									// 8
		builder.AppendAuxiliaryVector(AT_ENTRY, executable->EntryPoint);			// 9
		(AT_NOTELF);		// 10 - DO NOT IMPLEMENT
		(AT_UID);			// 11
		(AT_EUID);			// 12
		(AT_GID);			// 13
		(AT_EGID);			// 14
#ifdef _M_X64
		builder.AppendAuxiliaryVector(AT_PLATFORM, "x86_64");						// 15
#else
		builder.AppendAuxiliaryVector(AT_PLATFORM, "i686");							// 15
#endif
		(AT_HWCAP);			// 16
		(AT_CLKTCK);		// 17
		builder.AppendAuxiliaryVector(AT_SECURE, 0);								// 23
		(AT_BASE_PLATFORM);	// 24 - DO NOT IMPLEMENT
		builder.AppendAuxiliaryVector(AT_RANDOM, &pseudorandom, sizeof(GUID));		// 25
		(AT_HWCAP2);		// 26 - DO NOT IMPLEMENT
		(AT_EXECFN);		// 31
		(AT_SYSINFO);		// 32 - PROBABLY DO NOT IMPLEMENT
		builder.AppendAuxiliaryVector(AT_SYSINFO_EHDR, vdso->BaseAddress);			// 33

		// add exception handler from the system calls dll
		AddVectoredExceptionHandler(1, SysCallExceptionHandler);

		if(interpreter) interpreter->Execute(builder);
		else executable->Execute(builder);

		if(interpreter) delete interpreter;
		delete executable;
	}
	catch(Exception& ex) {
		MessageBox(NULL, ex, _T("Exception"), MB_OK | MB_ICONHAND);
		return (int)E_FAIL;
	}

	//ElfArguments::ReleaseArgumentStack(args);

	return 0;
}
