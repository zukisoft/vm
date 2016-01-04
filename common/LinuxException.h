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

#ifndef __LINUXEXCEPTION_H_
#define __LINUXEXCEPTION_H_
#pragma once

#include <exception>
#include <messages.h>
#include <Windows.h>

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
static inline constexpr HRESULT HRESULT_FROM_LINUX(int code)
{
	// Convert the code into an HRESULT value for Exception (0xE0000000 = SS|C bits)
	return (0xE0000000 | (FACILITY_LINUX << 16) | code);
}

//-----------------------------------------------------------------------------
// Class LinuxException
//
// Exception-based class used to wrap Linux system error codes

class LinuxException : public std::exception
{
public:

	// Instance Constructors
	//
	explicit LinuxException(int result);
	explicit LinuxException(int result, std::exception const& inner);

	// Copy Constructor
	//
	LinuxException(LinuxException const& rhs);

	// Destructor
	//
	virtual ~LinuxException();

	//-------------------------------------------------------------------------
	// std::exception implementation

	// what
	//
	// Gets a pointer to the exception message text
	virtual char const* what(void) const;
		
	//-------------------------------------------------------------------------
	// Properties

	// Code
	//
	// Exposes the Linux/POSIX result code
	__declspec(property(get=getCode)) int Code;
	int getCode(void) const;

	// InnerException
	//
	// Exposes a reference to the inner exception
	__declspec(property(get=getInnerException)) std::exception const& InnerException;
	std::exception const& getInnerException(void) const;

private:

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AllocateMessage
	//
	// Generates the formatted message string from the project resources
	static char* AllocateMessage(int result);

	//-------------------------------------------------------------------------
	// Member Variables

	int const					m_code;			// Linux/POSIX result code
	char* const					m_what;			// Formatted message text
	bool const					m_owned;		// Flag if message is owned
	std::exception const		m_inner;		// Inner exception reference

	// Fallback message text if the code could not be looked up in the messages
	static constexpr LPCTSTR s_defaultformat = TEXT("LinuxException code %1!d!");
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __LINUXEXCEPTION_H_