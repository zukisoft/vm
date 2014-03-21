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

extern "C" DWORD __stdcall ElfEntry(void* args);

typedef DWORD (*INITIALIZETLS)(const void* tlsbase, size_t tlslength);

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
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	AddVectoredExceptionHandler(0, UnhandledException);

	BOOL bresult = SymInitialize(GetCurrentProcess(), NULL, FALSE);

	// VDSO
	ElfImage* vdso = ElfImage::FromResource(MAKEINTRESOURCE(IDR_RCDATA_VDSO32INT80), RT_RCDATA);

 	HMODULE hm = LoadLibraryEx(L"D:\\GitHub\\vm\\out\\Win32\\Debug\\zuki.vm.syscalls32.dll", NULL, 0);
	INITIALIZETLS tlsinit = (INITIALIZETLS)GetProcAddress(hm, "InitializeTls");
	
	////DWORD result;
	//HANDLE h = CreateThread(NULL, 0, ElfEntry, (void*)0xFFFFEEEE, 0, NULL);
	//WaitForSingleObject(h, INFINITE);
	////GetExitCodeThread(h, &result);
	//CloseHandle(h);

	//return 0;

	ElfImage* p;
	ElfImage* pinterp;
	ElfArguments builder;
	builder.AppendArgument(L"hello world 123");
	builder.AppendArgument("hello world 456");

	builder.AppendEnvironmentVariable(L"hello", L"world");
	builder.AppendEnvironmentVariable(L"mike", L"brehm");
	builder.AppendEnvironmentVariable(L"reeve", L"skye");


	GUID pseudorandom;
	CoCreateGuid(&pseudorandom);

	//LDT_ENTRY ldtEntry;
	//DWORD selector = 0;
	//GetThreadSelectorEntry(GetCurrentThread(), selector, &ldtEntry);

	//uintptr_t fsVA = (ldtEntry.HighWord.Bytes.BaseHi) << 24 | (ldtEntry.HighWord.Bytes.BaseMid) << 16 | (ldtEntry.BaseLow);
 //


	//Elf32_Addr* args;
	//size_t count = builder.CreateArgumentStack(&args);

	try { 
		
		// note: would use a while loop to iterate over interpreters, they could be chained
		//p = ElfImage::FromFile(_T("D:\\Linux Binaries\\generic_x86\\system\\bin\\bootanimation"));
		//p = ElfImage::FromFile(_T("D:\\test"));
		//p = ElfImage::FromFile(_T("D:\\Linux Binaries\\generic_x86\\system\\bin\\linker"));
		//p = ElfImage::FromFile(_T("D:\\Linux Binaries\\busybox-x86"));
		p = ElfImage::FromFile(_T("D:\\Linux Binaries\\bionicapp"));
		//p = ElfImage::FromFile(_T("D:\\Linux Binaries\\generic_x86\\root\\init"));
		
		//LPCTSTR interp = p->Interpreter;

		//
		// AUXILIARY VECTORS
		//

		(AT_EXECFD);		// 2 - DO NOT IMPLEMENT
		
		if(p->ProgramHeaders) {

			builder.AppendAuxiliaryVector(AT_PHDR, p->ProgramHeaders);				// 3
			builder.AppendAuxiliaryVector(AT_PHENT, sizeof(Elf32_Phdr));			// 4
			builder.AppendAuxiliaryVector(AT_PHNUM, p->NumProgramHeaders);			// 5
		}

		builder.AppendAuxiliaryVector(AT_PAGESZ, MemoryRegion::PageSize);			// 6
		builder.AppendAuxiliaryVector(AT_BASE, p->BaseAddress);						// 7
		builder.AppendAuxiliaryVector(AT_FLAGS, 0);									// 8
		builder.AppendAuxiliaryVector(AT_ENTRY, p->EntryPoint);						// 9
		(AT_NOTELF);		// 10 - DO NOT IMPLEMENT
		(AT_UID);			// 11
		(AT_EUID);			// 12
		(AT_GID);			// 13
		(AT_EGID);			// 14
		builder.AppendAuxiliaryVector(AT_PLATFORM, "i686");							// 15
		(AT_HWCAP);			// 16
		(AT_CLKTCK);		// 17
		builder.AppendAuxiliaryVector(AT_SECURE, 0);								// 23
		(AT_BASE_PLATFORM);	// 24 - DO NOT IMPLEMENT
		builder.AppendAuxiliaryVector(AT_RANDOM, &pseudorandom, sizeof(GUID));		// 25
		(AT_HWCAP2);		// 26 - DO NOT IMPLEMENT
		(AT_EXECFN);		// 31
		(AT_SYSINFO);		// 32 - PROBABLY DO NOT IMPLEMENT
		builder.AppendAuxiliaryVector(AT_SYSINFO_EHDR, vdso->BaseAddress);

		if((tlsinit) && (p->TlsBaseAddress) && (p->TlsLength)) tlsinit(p->TlsBaseAddress, p->TlsLength);

		//AddVectoredExceptionHandler(1, SysCallHandler);
		p->Execute(builder);

		delete p;
		//delete pinterp;
	}
	catch(Exception& ex) {
		MessageBox(NULL, ex, _T("Exception"), MB_OK | MB_ICONHAND);
		return (int)E_FAIL;
	}

	//ElfArguments::ReleaseArgumentStack(args);

	return 0;
}

