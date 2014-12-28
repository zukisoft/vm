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

#ifndef __INDEXPOOL_H_
#define __INDEXPOOL_H_
#pragma once

#include <atomic>
#include <concurrent_queue.h>
#include <type_traits>
#include "Exception.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// IndexPool
//
// Implements a lock-free index pool with the potential for an extremely high
// upper boundary.  This type is most effective for pools where indexes tend to
// be released in small quantities and can be recycled aggressively, for example 
// file descriptor values or process/thread identifiers.
//
// Not recommended for situations where a large number of index values may be
// released and then not reallocated; this would cause the spent index queue
// to grow to ridiculous proportions and consume a great deal of memory.

template <typename _index_t>
class IndexPool
{
	// This class only works when _index_t is a signed integral data type
	static_assert(std::is_integral<_index_t>::value, "The data type for IndexPool must be a signed integral data type");
	static_assert(std::is_signed<_index_t>::value, "The data type for IndexPool must be a signed integral data type");

public:

	// Constructors
	//
	IndexPool() : IndexPool(0) {}
	IndexPool(_index_t reserved) : m_next(reserved) {}

	// Destructor
	//
	~IndexPool()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates an index from the pool
	_index_t Allocate(void)
	{
		_index_t index;				// Allocated index value

		// Try to use a spent index first before grabbing a new one; if the
		// return value overflowed, there are no more indexes left to use
		if(!m_spent.try_pop(index)) index = m_next++;
		if(index < 0) throw Exception(E_INDEXPOOL_EXHAUSTED);

		return index;
	}

	// Release
	//
	// Releases an index for re-use in the pool
	void Release(_index_t index)
	{
		// This class reuses indexes aggressively, push it into the spent queue
		// so that it be grabbed before allocating a new one
		m_spent.push(index);	
	}

private:

	IndexPool(const IndexPool&)=delete;
	IndexPool& operator=(const IndexPool&)=delete;

	// _queue_t
	//
	// Alias for the spent index queue data type
	using _queue_t = Concurrency::concurrent_queue<_index_t>;

	//-------------------------------------------------------------------------
	// Member Variables

	std::atomic<_index_t>	m_next;			// Next unused sequential index
	_queue_t				m_spent;		// Queue of spent indexes to reuse
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __INDEXPOOL_H_
