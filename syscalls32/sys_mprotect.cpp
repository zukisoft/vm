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

#include "stdafx.h"						// Include project pre-compiled headers

#pragma warning(push, 4)				// Enable maximum compiler warnings

#define PROT_READ       0x1             // page can be read
#define PROT_WRITE      0x2             // page can be written
#define PROT_EXEC       0x4             // page can be executed

inline static DWORD FlagsToProtection(DWORD flags)
{
	switch(flags) {

		case PROT_EXEC:								return PAGE_EXECUTE;
		case PROT_WRITE :							return PAGE_READWRITE;
		case PROT_READ :							return PAGE_READONLY;
		case PROT_EXEC | PROT_WRITE :				return PAGE_EXECUTE_READWRITE;
		case PROT_EXEC | PROT_READ :				return PAGE_EXECUTE_READ;
		case PROT_WRITE | PROT_READ :				return PAGE_READWRITE;
		case PROT_EXEC | PROT_WRITE | PROT_READ :	return PAGE_EXECUTE_READWRITE;
	}

	return PAGE_NOACCESS;
}

void sys_mprotect(PCONTEXT context)
{
	if(!VirtualProtect(reinterpret_cast<void*>(context->Ebx), 
		static_cast<size_t>(context->Ecx), 
		FlagsToProtection(context->Edx), 
		&context->Edx)) context->Eax = static_cast<DWORD>(-1);

	// TODO: NEED ERRNO CODES NOT -1 !!!!!!!!

	else context->Eax = 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
