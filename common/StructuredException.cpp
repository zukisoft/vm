//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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
#include "StructuredException.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// StructuredException::GetDefaultMessage (protected)
//
// Invoked when an HRESULT cannot be mapped to a message table string
//
// Arguments:
//
//	hresult		- Thrown HRESULT code

std::tstring StructuredException::GetDefaultMessage(HRESULT const& hresult)
{
	tchar_t buffer[256];			// Stack buffer to hold formatted string

	// HRESULTs here can either have been converted into WIN32 codes by RtlNtStatusToDosError
	// or can have remained the raw NTSTATUS codes if that function could not be mapped
	if(HRESULT_FACILITY(hresult) == FACILITY_WIN32) 
		_sntprintf_s(buffer, 256, _TRUNCATE, _T("Win32 system error code %d\r\n"), HRESULT_CODE(hresult));		
	else 
		_sntprintf_s(buffer, 256, _TRUNCATE, _T("NTSTATUS code 0x%08X\r\n"), hresult);

	return std::tstring(buffer);
}

//-----------------------------------------------------------------------------
// StructuredExcpetion::SeTranslator (static)
//
// SEH translator function that must be passed to _set_se_translator(), this
// is invoked to generate the corresponding C++ exception
//
// Arguments:
//
//	code		- SEH exception code
//	info		- SEH exception information

void StructuredException::SeTranslator(unsigned int code, LPEXCEPTION_POINTERS)
{
	// This function must not do anything other than throw the appropriate
	// C++ exception object based on the information provided
	throw StructuredException(code);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
