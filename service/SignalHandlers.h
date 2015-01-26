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

#ifndef __SIGNALHANDLERS_H_
#define __SIGNALHANDLERS_H_
#pragma once

#pragma warning(push, 4)

#include <memory>

//-----------------------------------------------------------------------------
// SignalHandlers
//
// Implements a collection of signal handlers that can be duplicated or
// shared among multiple process instances

class SignalHandlers
{
public:

	// Destructor
	//
	~SignalHandlers()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new default signal handler collection
	static std::shared_ptr<SignalHandlers> Create(void);

	// Duplicate (static)
	//
	// Duplicates the handle collection into a new instance
	static std::shared_ptr<SignalHandlers> Duplicate(const std::shared_ptr<SignalHandlers>& existing);

private:

	SignalHandlers(const SignalHandlers&)=delete;
	SignalHandlers& operator=(const SignalHandlers&)=delete;

	//-------------------------------------------------------------------------
	// Instance Constructors
	//
	SignalHandlers() {}
	friend class std::_Ref_count_obj<SignalHandlers>;

	//-------------------------------------------------------------------------
	// Member Variables
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SIGNALHANDLERS_H_
