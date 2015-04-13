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

#ifndef __PIDNAMESPACE_H_
#define __PIDNAMESPACE_H_
#pragma once

#include <atomic>
#include <memory>
#include <concurrent_queue.h>
#include <linux/types.h>
#include "Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// PidNamespace
//
// Implements a process identifier namespace

class PidNamespace
{
public:

	// Destructor
	//
	~PidNamespace()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates a new pid_t from the pool
	uapi::pid_t Allocate(void);

	// Create
	//
	// Creates a new PidNamespace instance
	static std::shared_ptr<PidNamespace> Create(void);

	// Release
	//
	// Releases a previously allocated pid_t
	void Release(uapi::pid_t pid);

private:

	PidNamespace(const PidNamespace&)=delete;
	PidNamespace& operator=(const PidNamespace&)=delete;

	// spent_queue_t
	//
	// Alias for the spent index queue data type
	using spent_queue_t = Concurrency::concurrent_queue<uapi::pid_t>;

	// Instance Constructor
	//
	PidNamespace()=default;
	friend class std::_Ref_count_obj<PidNamespace>;

	//-------------------------------------------------------------------------
	// Member Variables

	std::atomic<uapi::pid_t>	m_next = 1;		// Next unused sequential pid
	spent_queue_t				m_spent;		// Queue of spent pids to reuse
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PIDNAMESPACE_H_
