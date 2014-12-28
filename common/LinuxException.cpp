//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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
#include "LinuxException.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// LinuxException::GetDefaultMessage (protected)
//
// Invoked when an HRESULT cannot be mapped to a message table string
//
// Arguments:
//
//	hresult		- Thrown HRESULT code

std::tstring LinuxException::GetDefaultMessage(const HRESULT& hresult)
{
	// Technically anything that gets in here came from HRESULT_FROM_LINUX(), but in case not ...
	if(HRESULT_FACILITY(hresult) != FACILITY_LINUX) return Exception::GetDefaultMessage(hresult);

	tchar_t buffer[256];			// Stack buffer to hold formatted string

	// Format a default message for the HRESULT that just shows the result code as an integer
	_sntprintf_s(buffer, 256, _TRUNCATE, _T("Linux system error code %d\r\n"), HRESULT_CODE(hresult));
	return std::tstring(buffer);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
