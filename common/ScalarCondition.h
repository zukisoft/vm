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

#ifndef __SCALARCONDITION_H_
#define __SCALARCONDITION_H_
#pragma once

#include <condition_variable>
#include <mutex>

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// ScalarCondition
//
// Implements a simple scalar value condition variable

template <typename _type>
class ScalarCondition
{
public:

	// Instance Constructor
	//
	explicit ScalarCondition(_type initial) : m_value(initial) {}

	// Destructor
	//
	~ScalarCondition()=default;

	// assignment operator
	//
	const ScalarCondition& operator=(const _type& value) 
	{
		std::unique_lock<std::mutex> critsec(m_lock);

		// Change the stored value and wake up any threads waiting for it,
		// the predicate in WaitUntil handles spurious or no-change wakes
		m_value = value;
		m_condition.notify_all();

		return *this;
	}

	//-------------------------------------------------------------------------
	// Member Functions

	// WaitUntil
	//
	// Waits until the value has been set to the specified value
	void WaitUntil(const _type& value)
	{
		std::unique_lock<std::mutex> critsec(m_lock);

		// If the value does not already match the provided value, wait for it
		if(m_value == value) return;
		m_condition.wait(critsec, [&]() -> bool { return m_value == value; });
	}

private:

	ScalarCondition(const ScalarCondition&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	std::condition_variable		m_condition;	// Condition variable
	std::mutex					m_lock;			// Synchronization object
	_type						m_value;		// Contained value
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SCALARCONDITION_H_
