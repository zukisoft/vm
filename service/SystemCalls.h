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

#ifndef __SYSTEMCALLS_H_
#define __SYSTEMCALLS_H_
#pragma once

#include "Process.h"

#pragma warning(push, 4)			

//-----------------------------------------------------------------------------
// UUID_SYSTEMCALLS32
//
// Type UUID for the 32-bit system calls implementation; this UUID is transparent
// to the client application(s)

// {D967A755-869F-4180-A9C0-BA96D7B41E18}
__declspec(selectany) extern UUID UUID_SYSTEMCALLS32 = 
{ 0xd967a755, 0x869f, 0x4180, { 0xa9, 0xc0, 0xba, 0x96, 0xd7, 0xb4, 0x1e, 0x18 } };

//-----------------------------------------------------------------------------
// UUID_SYSTEMCALLS64
//
// Type UUID for the 64-bit system calls implementation; this UUID is transparent
// to the client application(s)

// {94F810E2-56FE-4FAB-A0A6-2F631C807036}
__declspec(selectany) extern UUID UUID_SYSTEMCALLS64 = 
{ 0x94f810e2, 0x56fe, 0x4fab, { 0xa0, 0xa6, 0x2f, 0x63, 0x1c, 0x80, 0x70, 0x36 } };

//-----------------------------------------------------------------------------
// Interface SystemCalls
//
// Build-specific (32bit vs 64bit) implementation of the system calls interface;
// this will be invoked from the RPC entry points

struct __declspec(novtable) SystemCalls
{
	virtual __int3264 SysAttachProcess(DWORD processid, Process** process) = 0;

	virtual __int3264 SysClose(Process* process, int fd) = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSTEMCALLS_H_
