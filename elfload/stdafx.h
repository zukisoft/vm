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
#include <windows.h>			// Include main Windows declarations
#include <tchar.h>				// Include generic text mappings
#include <stdarg.h>				// Include standard argument decls (va_list)
#include <stdint.h>				// Include standard integer declarations
#include <stdlib.h>				// Include standard library declarations

// Debug Help Library
#include <DbgHelp.h>			// Include Debug Helper library decarlations
#pragma comment(lib, "DbgHelp.lib")

// STL
#include <algorithm>			// for_each()
#include <functional>			// lambdas
#include <memory>				// unique_ptr<>, shared_ptr<>, etc.
#include <string>				// string<>, wstring<>, etc.
#include <vector>				// vector<> template declarations

// KiB / MiB / GiB

#define KiB		*(1 << 10)		// KiB multiplier
#define MiB		*(1 << 20)		// MiB multiplier
#define GiB		*(1 << 30)		// GiB multiplier

namespace std {
#ifdef _UNICODE
	typedef wstring tstring;
#else
	typedef string tstring;
#endif
}

#ifdef _UNICODE
typedef wchar_t tchar_t;
#else
typedef char tchar_t;
#endif

// Message Resources
#include <messages.h>

//-----------------------------------------------------------------------------

#endif	// __STDAFX_H_
