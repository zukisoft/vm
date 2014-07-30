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

#ifndef __STDAFX_H_
#define __STDAFX_H_
#pragma once

//-----------------------------------------------------------------------------
// Win32 Declarations

#define NTDDI_VERSION			NTDDI_WIN7
#define	_WIN32_WINNT			_WIN32_WINNT_WIN7
#define WINVER					_WIN32_WINNT_WIN7
#define	_WIN32_IE				_WIN32_IE_IE80

// Windows / CRT
#include <windows.h>
#include <rpc.h>
#include <stdint.h>
#include <memory>
#include <string>

#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "rpcns4.lib")

// KiB / MiB / GiB

#define KiB		*(1 << 10)		// KiB multiplier
#define MiB		*(1 << 20)		// MiB multiplier
#define GiB		*(1 << 30)		// GiB multiplier

// Generic Text Mappings
#include <generic_text.h>

// Linux
#include <linux/types.h>
#include <linux/errno.h>

//---------------------------------------------------------------------------
// Service Template Library

#include <servicelib.h>

#include <vm.service.h>
#include <messages.h>

//---------------------------------------------------------------------------
// Project COM Declarations

//#include <initguid.h>			// We need DECLARE_GUID support for the CLSIDs

//-----------------------------------------------------------------------------

#endif	// __STDAFX_H_
