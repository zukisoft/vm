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

#include "stdafx.h"
#include "Pid.h"

// Forward Declarations
//
#include "Namespace.h"
#include "PidNamespace.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Pid Destructor

Pid::~Pid()
{
	// Iterate over all allocated pid_ts for this Pid instance and release them
	for(const auto& iterator : m_pids) iterator.first->Release(iterator.second);
}

//-----------------------------------------------------------------------------
// Pid::getValue
//
// Retrieves the pid_t value associated with a specific namespace

uapi::pid_t Pid::getValue(const std::shared_ptr<Namespace>& ns) const
{
	return getValue(ns->Pid);		// Call through the Namespace
}

//-----------------------------------------------------------------------------
// Pid::getValue
//
// Retrieves the pid_t value associated with a specific namespace

uapi::pid_t Pid::getValue(const std::shared_ptr<PidNamespace>& ns) const
{
	try { return m_pids.at(ns); }
	catch(...) { throw std::exception("todo"); } // todo: exception
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
