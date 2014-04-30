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

#ifndef __EXCEPTION_H_
#define __EXCEPTION_H_
#pragma once

#include "char_t.h"
#include "tstring.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// Exception
//
// Exception class. Use Exception::SetMessagesModule() to specify an HMODULE
// where any non-system RT_MESSAGETABLE resources are located; by default the
// calling process executable will be checked for these resources.

class Exception
{
public:

	// Instance Constructors
	//
	Exception(HRESULT result, ...);
	Exception(Exception& inner, HRESULT result, ...);
	Exception(const Exception& rhs);

	// Destructor
	//
	virtual ~Exception();

	//-------------------------------------------------------------------------
	// Overloaded Operators

	// const tchar_t* conversion
	//
	operator const tchar_t*() const { return m_message.c_str(); }

	// HRESULT conversion
	//
	operator HRESULT() const { return m_result; }

	// assignment
	//
	Exception& operator=(const Exception& rhs);

	//-------------------------------------------------------------------------
	// Member Functions

	// SetMessagesModule
	//
	// Alters the module handle used to look up message table strings
	static HMODULE SetMessagesModule(HMODULE module);

	//-------------------------------------------------------------------------
	// Properties

	// Code
	//
	// Exposes the underlying error code
	__declspec(property(get=getCode)) HRESULT Code;
	HRESULT getCode(void) const { return m_result; }

	// InnerException
	//
	// Exposes the inner exception instance
	__declspec(property(get=getInnerException)) Exception* InnerException;
	Exception* getInnerException(void) const { return m_inner; }

	// Message
	//
	// Exposes the exception message string
	__declspec(property(get=getMessage)) const tchar_t* Message;
	const tchar_t* getMessage(void) const { return m_message.c_str(); }

private:

	//-------------------------------------------------------------------------
	// Member Variables

	HRESULT					m_result;			// Error HRESULT code
	std::tstring			m_message;			// Formatted message string
	Exception*				m_inner;			// Inner exception object
	static HMODULE			s_module;			// Resource module handle
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EXCEPTION_H_
