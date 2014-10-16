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
#include <linux/types.h>

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
	PathIterator(const uapi::char_t* path);
	~PathIterator()=default;

	// operator bool
	//
	// Indicates if the current component pointer is non-empty
	operator bool() const { return ((m_current) && (*m_current)); }

	// operator !
	//
	// Indicates if the current component pointer is empty
	bool operator !() const { return ((m_current == nullptr) || (*m_current == 0)); }

	// Increment operator
	//
	// Adjusts the current pointer to the next path component
	PathIterator& operator++();

	//-------------------------------------------------------------------------
	// Properties

	// Consumed
	//
	// Returns a pointer to the consumed path components
	__declspec(property(get=getConsumed)) const uapi::char_t* Consumed;
	const uapi::char_t* getConsumed(void) const { return m_consumed; }

	// Current
	//
	// Returns a pointer to the current component in the path
	__declspec(property(get=getCurrent)) const uapi::char_t* Current;
	const uapi::char_t* getCurrent(void) const { return m_current; }

	// Remaining
	//
	// Returns a pointer to the remaining path components
	__declspec(property(get=getRemaining)) const uapi::char_t* Remaining;
	const uapi::char_t* getRemaining(void) const { return m_remaining; }

private:

	PathIterator(const PathIterator&)=delete;
	PathIterator& operator=(const PathIterator&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	std::vector<uapi::char_t>	m_path;				// Path string vector
	uapi::char_t*				m_consumed;			// Pointer to the consumed data
	uapi::char_t*				m_current;			// Pointer to the current component
	uapi::char_t*				m_remaining;		// Pointer to the remaining data
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PATHITERATOR_H_
