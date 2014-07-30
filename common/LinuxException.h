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

#ifndef __LINUXEXCEPTION_H_
#define __LINUXEXCEPTION_H_
#pragma once

#include "generic_text.h"
#include "Exception.h"
#include <messages.h>

#pragma warning(push, 4)

// FACILITY_LINUX
//
// LinuxException facility code, should be declared in the included message file header
#ifndef FACILITY_LINUX
#error LinuxException requires FACILITY_LINUX be defined in the included message file header
#endif

// HRESULT_FROM_LINUX
//
// Creates an HRESULT from a linux error code
static inline HRESULT HRESULT_FROM_LINUX(int code)
{
	// Convert the code into an HRESULT value for Exception (0xE0000000 = S|R|C bits)
	return (0xE0000000 | (FACILITY_LINUX << 16) | code);
}

//-----------------------------------------------------------------------------
// Class LinuxException
//
// Exception-based class used to wrap Linux system error codes.  Insertion
// arguments are not supported as none of these messages will have inserts

class LinuxException : public Exception
{
public:

	// Instance Constructor (int)
	//
	LinuxException(const int& result) : Exception(HRESULT_FROM_LINUX(result)) {}

	// Instance Constructor (int + Inner Exception)
	//
	LinuxException(const int& result, const Exception& inner) : Exception(HRESULT_FROM_LINUX(result), inner) {}

	// Instance Constructor (int + HMODULE)
	//
	LinuxException(const int& result, const HMODULE& module) : Exception(HRESULT_FROM_LINUX(result), module) {}

	// Instance Constructor (int + HMODULE, Inner Exception)
	//
	LinuxException(const int& result, const HMODULE& module, const Exception& inner) : Exception(HRESULT_FROM_LINUX(result), module, inner) {}

	// Destructor
	//
	virtual ~LinuxException()=default;

protected:

	// GetDefaultMessage (Exception)
	//
	// Invoked when an HRESULT code cannot be mapped to a message table string
	virtual std::tstring GetDefaultMessage(const HRESULT& hresult);

private:

	LinuxException()=delete;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __LINUXEXCEPTION_H_