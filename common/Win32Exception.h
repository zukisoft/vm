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

#include <exception>
#include <Windows.h>

#pragma warning(push, 4)	

//-----------------------------------------------------------------------------
// Class Win32Exception
//
// Exception-based class used to wrap Windows system error codes

class Win32Exception : public std::exception
{
public:

	// Instance Constructors
	//
	Win32Exception();
	explicit Win32Exception(DWORD result);

	// Copy Constructor
	//
	Win32Exception(Win32Exception const& rhs);

	// Destructor
	//
	virtual ~Win32Exception();

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
	// Exposes the Windows system error code
	__declspec(property(get=getCode)) DWORD Code;
	DWORD getCode(void) const;

private:

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AllocateMessage
	//
	// Generates the formatted message string from the project resources
	static char* AllocateMessage(DWORD result);

	//-------------------------------------------------------------------------
	// Member Variables

	DWORD const					m_code;			// Windows system error code
	char* const					m_what;			// Formatted message text
	bool const					m_owned;		// Flag if message is owned

	// Fallback message text if the code could not be looked up
	static constexpr LPCTSTR s_defaultformat = TEXT("Win32Exception code %1!lu!");
};


//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __WIN32EXCEPTION_H_
