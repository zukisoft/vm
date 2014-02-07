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

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Exception
//
// Exception class

class Exception
{
public:

	// Constructor
	//
	explicit Exception(HRESULT hResult, ...);

	// Copy Constructor
	//
	Exception(const Exception& rhs) : m_hResult(rhs.m_hResult), m_message(rhs.m_message) {}

	//-------------------------------------------------------------------------
	// Overloaded Operators

	operator LPCTSTR() const { return m_message.c_str(); }
	Exception& operator=(const Exception& rhs);

private:

	//-------------------------------------------------------------------------
	// Member Variables

	HRESULT					m_hResult;			// Error HRESULT code
	std::tstring			m_message;			// Formatted message string
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EXCEPTION_H_
