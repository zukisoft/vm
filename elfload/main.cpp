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

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	//DWORD result;
	//HANDLE h = CreateThread(NULL, 0, ElfEntry, (void*)0xFEEDFEEDFEEDFEED, 0, NULL);
	//WaitForSingleObject(h, INFINITE);
	//GetExitCodeThread(h, &result);
	//CloseHandle(h);

	//return 0;

	ElfImage* p;
	ElfImage* pinterp;
	try { 
		
		// note: would use a while loop to iterate over interpreters, they could be chained
		p = ElfImage::Load(_T("D:\\Linux Binaries\\generic_x86\\system\\bin\\bootanimation"));
		pinterp = ElfImage::Load(_T("D:\\Linux Binaries\\generic_x86\\system\\bin\\linker"));
		
		LPCTSTR interp = p->Interpreter;

		uint32_t result = pinterp->Execute(nullptr);

		delete p;
		delete pinterp;
	}
	catch(Exception& ex) {
		MessageBox(NULL, ex, _T("Exception"), MB_OK | MB_ICONHAND);
		return (int)E_FAIL;
	}

	return 0;
}

