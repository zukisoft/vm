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

#ifndef __INDEXPOOL2_H_
#define __INDEXPOOL2_H_
#pragma once

#include <stdint.h>
#include "Bitmap.h"
#include "Exception.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// IndexPool2
//
// Implements a bitmap-based fixed length index pool.  This type is most effective 
// for relatively small fixed-length pools that tend to be allocated and released 
// sequentially, or require the ability to be copied/moved.  Performance of this
// pool is hindered by the need for a critical section to protect the bitmap object.

class IndexPool2
{
public:

	// Constructors
	//
	IndexPool2(uint32_t poolsize) : IndexPool2(poolsize, 0) {}
	IndexPool2(uint32_t poolsize, uint32_t reserved) : m_bitmap(poolsize), m_reserved(reserved) 
	{
		// As ridiculous as it would seem to create a bitmapped pool large enough to hold 4 billion values,
		// it would need 512MB, verify that an overflow won't occur when applying the reservation offset
		if((UINT32_MAX - reserved) <= poolsize) throw Exception(E_BOUNDS);
	}

	// Destructor
	//
	~IndexPool2()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates an index from the pool
	uint32_t Allocate(void)
	{
		sync::critical_section::scoped_lock critsec(m_lock);

		// Locate a single clear bit in the bitmap and set it
		uint32_t result = m_bitmap.FindClearAndSet();
		if(result == Bitmap::NotFound) throw Exception(E_INDEXPOOL_EXHAUSTED);

		// Apply the reservation offset to generate the index
		return m_reserved + result;
	}

	// Release
	//
	// Releases an index for re-use in the pool
	void Release(uint32_t index)
	{
		sync::critical_section::scoped_lock critsec(m_lock);

		// Remove the reservation offset from the index and clear that bit
		m_bitmap.Clear(index - m_reserved);
	}

private:

	IndexPool2(const IndexPool2&)=delete;
	IndexPool2& operator=(const IndexPool2&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	uint32_t						m_reserved;		// Number of reserved indexes
	Bitmap							m_bitmap;		// Contained Bitmap instance
	mutable sync::critical_section	m_lock;			// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __INDEXPOOL2_H_
