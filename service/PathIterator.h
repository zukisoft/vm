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

#ifndef __PATHITERATOR_H_
#define __PATHITERATOR_H_
#pragma once

#include <vector>
#include "generic_text.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// PathIterator
//
// Implements a forward-only iterator over a POSIX path component string

class PathIterator
{
public:

	// Constructor / Destructor
	//
	PathIterator(const tchar_t* path);
	~PathIterator()=default;

	// operator bool
	//
	// Indicates if the current component pointer is valid
	operator bool() const { return ((m_current) && (*m_current)); }

	// Increment operator
	//
	// Adjusts the current pointer to the next path component
	PathIterator& operator++();

	//-------------------------------------------------------------------------
	// Properties

	// Consumed
	//
	// Returns a pointer to the consumed path components
	__declspec(property(get=getConsumed)) const tchar_t* Consumed;
	const tchar_t* getConsumed(void) const { return m_consumed; }

	// Current
	//
	// Returns a pointer to the current component in the path
	__declspec(property(get=getCurrent)) const tchar_t* Current;
	const tchar_t* getCurrent(void) const { return m_current; }

	// Remaining
	//
	// Returns a pointer to the remaining path components
	__declspec(property(get=getRemaining)) const tchar_t* Remaining;
	const tchar_t* getRemaining(void) const { return m_remaining; }

private:

	PathIterator(const PathIterator&)=delete;
	PathIterator& operator=(const PathIterator&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	std::vector<tchar_t>	m_path;				// Path string vector
	tchar_t*				m_consumed;			// Pointer to the consumed data
	tchar_t*				m_current;			// Pointer to the current component
	tchar_t*				m_remaining;		// Pointer to the remaining data
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PATHITERATOR_H_
