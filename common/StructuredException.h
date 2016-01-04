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

#ifndef __STRUCTUREDEXCEPTION_H_
#define __STRUCTUREDEXCEPTION_H_
#pragma once

#include <functional>
#include <Windows.h>
#include "Exception.h"
#include "NtApi.h"
#include "generic_text.h"

#pragma warning(push, 4)	

//-----------------------------------------------------------------------------
// Class StructuredException
//
// Exception-based class used to translate SEH codes into C++ exceptions.  The
// function _set_se_translator() must be called in main/DllMain, passing it the
// static StructuredException::Translator to activate this class.
//
// When possible, the low level NTSTATUS codes will be mapped to their corresponding 
// Win32 error codes.  This was done mainly because the message table resources in 
// NTDLL.DLL that have insertions are generally incompatible with FormatMessage().

class StructuredException : public Exception
{
public:

	// Instance Constructor (NTSTATUS)
	//
	StructuredException(NTSTATUS const& status) : 
		Exception{ HRESULT_FROM_WIN32(NtApi::RtlNtStatusToDosError(status)) } {}

	// Instance Constructor (NTSTATUS + Inner Exception)
	//
	StructuredException(NTSTATUS const& status, Exception const& inner) :
		Exception{ HRESULT_FROM_WIN32(NtApi::RtlNtStatusToDosError(status)), inner } {}

	// Destructor
	//
	virtual ~StructuredException()=default;

	// SeTranslator (static)
	//
	// Static handler to pass to _set_se_translator() to active this class
	static void SeTranslator(unsigned int code, LPEXCEPTION_POINTERS);

protected:

	// GetDefaultMessage (Exception)
	//
	// Invoked when an HRESULT code cannot be mapped to a message table string
	virtual std::tstring GetDefaultMessage(HRESULT const& hresult);
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __STRUCTUREDEXCEPTION_H_
