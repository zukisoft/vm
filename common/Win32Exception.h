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

#ifndef __WIN32EXCEPTION_H_
#define __WIN32EXCEPTION_H_
#pragma once

#include <Windows.h>
#include "Exception.h"
#include "generic_text.h"

#pragma warning(push, 4)	

//-----------------------------------------------------------------------------
// Class Win32Exception
//
// Exception-based class used to wrap Windows system error codes.  Insertion
// arguments are not supported nor is a module handle as these messages are
// system defined and almost never have useful insertions

class Win32Exception : public Exception
{
public:

	// Instance Constructor
	//
	Win32Exception() : Exception(HRESULT_FROM_WIN32(GetLastError())) {}

	// Destructor
	//
	virtual ~Win32Exception()=default;

	// Instance Constructor (Inner Exception)
	//
	Win32Exception(Exception const& inner) : Exception(HRESULT_FROM_WIN32(GetLastError()), inner) {}

	// Instance Constructor (DWORD)
	//
	Win32Exception(DWORD const& result) : Exception(HRESULT_FROM_WIN32(result)) {}

	// Instance Constructor (DWORD + Inner Exception)
	//
	Win32Exception(DWORD const& result, Exception const& inner) : Exception(HRESULT_FROM_WIN32(result), inner) {}

protected:

	// GetDefaultMessage (Exception)
	//
	// Invoked when an HRESULT code cannot be mapped to a message table string
	virtual std::tstring GetDefaultMessage(HRESULT const& hresult);
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __WIN32EXCEPTION_H_
