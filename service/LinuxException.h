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

#include <map>
#include "char_t.h"
#include "tstring.h"
#include "Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// LinuxException
//
// Exception thrown using a Linux error code

class LinuxException : public Exception
{
public:

	LinuxException(int err);
	LinuxException(int err, DWORD win32);
	LinuxException(int err, HRESULT win32);

private:

	struct ErrorMapEntry
	{
		int				code;
		const char_t* 	name;
		const char_t*	message;
	};

	static const ErrorMapEntry s_map[];
};

	//// svctl::winexception
	////
	//// specialization of std::exception for Win32 error codes
	//class winexception : public std::exception
	//{
	//public:
	//	
	//	// Instance Constructors
	//	explicit winexception(DWORD result);
	//	explicit winexception(HRESULT hresult) : winexception(static_cast<DWORD>(hresult)) {}
	//	winexception() : winexception(GetLastError()) {}

	//	// Destructor
	//	virtual ~winexception()=default;

	//	// code
	//	//
	//	// Exposes the Win32 error code used to construct the exception
	//	DWORD code() const { return m_code; }

	//	// std::exception::what
	//	//
	//	// Exposes a string-based representation of the exception (ANSI only)
	//	virtual const char_t* what() const { return m_what.c_str(); }

	//private:

	//	// m_code
	//	//
	//	// Underlying error code
	//	DWORD m_code;

	//	// m_what
	//	//
	//	// Exception message string derived from the Win32 error code
	//	std::string m_what;
	//};


//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __LINUXEXCEPTION_H_