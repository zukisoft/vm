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

#ifndef __PIDNAMESPACE_H_
#define __PIDNAMESPACE_H_
#pragma once

#include <atomic>
#include <concurrent_queue.h>
#include <memory>
#include "Pid.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// PidNamespace
//
// Provides an isolated process id number space. See pid_namespace(7).

class PidNamespace : public std::enable_shared_from_this<PidNamespace>
{
friend class Pid;
public:

	// Destructor
	//
	~PidNamespace()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Creates a new Pid instance from this namespace and all ancestors
	std::shared_ptr<Pid> Allocate(void);

	// Create (static)
	//
	// Creates a new PidNamespace instance
	static std::shared_ptr<PidNamespace> Create(void);
	static std::shared_ptr<PidNamespace> Create(const std::shared_ptr<PidNamespace>& ancestor);

private:

	PidNamespace(const PidNamespace&)=delete;
	PidNamespace& operator=(const PidNamespace&)=delete;

	// spentqueue_t
	//
	// Concurrency-safe queue of spent pid_ts for reuse
	using spentqueue_t = Concurrency::concurrent_queue<uapi::pid_t>;

	// Instance Constructor
	//
	PidNamespace(const std::shared_ptr<PidNamespace>& ancestor);
	friend class std::_Ref_count_obj<PidNamespace>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AllocatePid
	//
	// Allocates a raw pid_t from this namespace pid_t pool
	uapi::pid_t AllocatePid(void);

	// ReleasePid
	//
	// Releases a pid_t that has been allocated in this namespace
	void ReleasePid(uapi::pid_t pid);

	//-------------------------------------------------------------------------
	// Member Variables

	std::atomic<uapi::pid_t>		m_next = 1;		// Next unused sequential pid_t
	spentqueue_t					m_spent;		// Queue of spent pid_ts for reuse
	std::weak_ptr<PidNamespace>		m_ancestor;		// PidNamespace ancestor
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PIDNAMESPACE_H_
