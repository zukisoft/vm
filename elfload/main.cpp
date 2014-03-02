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

extern "C" DWORD __stdcall ElfEntry(void* args);
//LPTHREAD_START_ROUTINE

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
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

	builder.AppendAuxiliaryVector(AT_PLATFORM, L"i686");

	//Elf32_Addr* args;
	//size_t count = builder.CreateArgumentStack(&args);

	try { 
		
		// note: would use a while loop to iterate over interpreters, they could be chained
		//p = ElfImage::Load(_T("D:\\Linux Binaries\\generic_x86\\system\\bin\\bootanimation"));
		p = ElfImage::Load(_T("D:\\test"));
		///pinterp = ElfImage::Load(_T("D:\\Linux Binaries\\generic_x86\\system\\bin\\linker"));
		
		//LPCTSTR interp = p->Interpreter;

		uint32_t result = p->Execute(builder);

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

