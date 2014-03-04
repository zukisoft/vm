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
#include "Exception.h"
#include "ElfImage.h"

DWORD WINAPI ThreadProc(void* args)
{
	memset(_alloca(64), 0xDD, 64);
	return 0xBB;
}

#define LINUX_ENOSYS		38

static HMODULE syscalls;

typedef void (*syscall_fn)(PCONTEXT context);

//-----------------------------------------------------------------------------
// SysCallHandler
//
// Intercepts and processes a 32-bit Linux system call using a vectored exception
// handler.  Technique is based on a sample presented by proog128 available at:
// http://0xef.wordpress.com/2012/11/17/emulate-linux-system-calls-on-windows/
//
// Arguments:
//
//	exception		- Exception information

LONG CALLBACK SysCallHandler(PEXCEPTION_POINTERS exception)
{
	// INT 0x80 instruction should cause an access violation
	if(exception->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_CONTINUE_SEARCH;

	// EIP --> 0xCD, 0x80		; INT 0x80
	if(*reinterpret_cast<uint16_t*>(exception->ContextRecord->Eip) != 0x80CD)
		return EXCEPTION_CONTINUE_SEARCH;

	// CALL # = eax
	//uint32_t syscall = exception->ContextRecord->Eax;

	syscall_fn syscall = reinterpret_cast<syscall_fn>(GetProcAddress(syscalls, reinterpret_cast<LPCSTR>(exception->ContextRecord->Eax)));
	if(!syscall) exception->ContextRecord->Eax = LINUX_ENOSYS;
	else syscall(exception->ContextRecord);

	exception->ContextRecord->Eip += 2;
	return EXCEPTION_CONTINUE_EXECUTION;
}

extern "C" DWORD __stdcall ElfEntry(void* args);
//LPTHREAD_START_ROUTINE



int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	syscalls = LoadLibraryEx(L"D:\\GitHub\\vm\\out\\Win32\\Debug\\zuki.vm.syscalls32.dll", NULL, 0);
	
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
	builder.AppendEnvironmentVariable(L"mike", nullptr);
	builder.AppendEnvironmentVariable(L"reeve", L"skye");


	GUID pseudorandom;
	CoCreateGuid(&pseudorandom);

	//Elf32_Addr* args;
	//size_t count = builder.CreateArgumentStack(&args);

	try { 
		
		// note: would use a while loop to iterate over interpreters, they could be chained
		//p = ElfImage::Load(_T("D:\\Linux Binaries\\generic_x86\\system\\bin\\bootanimation"));
		//p = ElfImage::Load(_T("D:\\test"));
		//p = ElfImage::Load(_T("D:\\Linux Binaries\\generic_x86\\system\\bin\\linker"));
		p = ElfImage::Load(_T("D:\\Linux Binaries\\busybox-x86"));
		
		//LPCTSTR interp = p->Interpreter;

		//
		// AUXILIARY VECTORS
		//

		(AT_EXECFD);		// 2
		
		if(p->ProgramHeaders) {

			builder.AppendAuxiliaryVector(AT_PHDR, p->ProgramHeaders);				// 3
			builder.AppendAuxiliaryVector(AT_PHENT, sizeof(Elf32_Phdr));			// 4
			builder.AppendAuxiliaryVector(AT_PHNUM, p->NumProgramHeaders);			// 5
		}

		builder.AppendAuxiliaryVector(AT_PAGESZ, MemoryRegion::PageSize);			// 6
		builder.AppendAuxiliaryVector(AT_BASE, p->BaseAddress);						// 7
		(AT_FLAGS);			// 8
		builder.AppendAuxiliaryVector(AT_ENTRY, p->EntryPoint);						// 9
		// 10
		(AT_UID);			// 11
		(AT_EUID);			// 12
		(AT_GID);			// 13
		(AT_EGID);			// 14
		builder.AppendAuxiliaryVector(AT_PLATFORM, "i686");							// 15
		(AT_HWCAP);			// 16
		(AT_CLKTCK);		// 17
		builder.AppendAuxiliaryVector(AT_SECURE, 0);								// 23
		(AT_BASE_PLATFORM);	// 24
		builder.AppendAuxiliaryVector(AT_RANDOM, &pseudorandom, sizeof(GUID));		// 25
		(AT_HWCAP2);		// 26
		(AT_EXECFN);		// 31
		(AT_SYSINFO);		// 32
		(AT_SYSINFO_EHDR);	// 33

		AddVectoredExceptionHandler(1, SysCallHandler);
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

