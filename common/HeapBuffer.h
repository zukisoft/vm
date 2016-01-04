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

#ifndef __HEAPBUFFER_H_
#define __HEAPBUFFER_H_
#pragma once

#include "Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// HeapBuffer
//
// Implements a fixed-length heap buffer; supports copy and move construction

template <typename _type>
class HeapBuffer
{
public:

	// Constructor
	//
	HeapBuffer(size_t elements = 1) : m_count(elements)
	{ 
		// Cannot call operator new[] with 0 elements to allocate
		if(elements == 0) throw Exception(E_BOUNDS);

		// Allocate the heap buffer with operator new[]
		m_buffer = new _type[elements]; 
		if(!m_buffer) throw Exception(E_OUTOFMEMORY); 
	}

	// Move Constructor
	//
	HeapBuffer(HeapBuffer&& rhs) : m_count(rhs.m_count), m_buffer(rhs.m_buffer)
	{
		rhs.m_count = 0;
		rhs.m_buffer = nullptr;
	}

	// Destructor
	//
	~HeapBuffer() { if(m_buffer) delete[] m_buffer; }

	//-------------------------------------------------------------------------
	// Overloaded Operators
	
	// _type* conversion operator
	//
	operator _type*() const { return m_buffer; }

	// Address-of operator
	//
	_type* operator&() const { return m_buffer; }

	// Array subscript operator
	//
	_type& operator[] (int index) 
	{ 
		if((index < 0) || (static_cast<size_t>(index) >= m_count)) throw Exception(E_BOUNDS);
		return m_buffer[index]; 
	}

	// Array subscript operator
	//
	_type& operator[] (size_t index) 
	{ 
		if(index < m_count) return m_buffer[index]; 
		else throw Exception(E_BOUNDS);
	}

	//-------------------------------------------------------------------------
	// Properties

	// Count
	//
	// Gets the number of allocated elements
	__declspec(property(get=getCount)) size_t Count;
	size_t getCount(void) const { return m_count; }

	// Size
	//
	// Gets the size of the buffer, in bytes
	__declspec(property(get=getSize)) size_t Size;
	size_t getSize(void) const { return (m_count * sizeof(_type)); }

private:

	HeapBuffer(const HeapBuffer&)=delete;
	HeapBuffer& operator=(const HeapBuffer&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	size_t			m_count = 0;				// Number of allocated objects
	_type*			m_buffer = nullptr;			// Allocated heap memory pointer
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HEAPBUFFER_H_
