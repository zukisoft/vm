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

#ifndef __PID_H_
#define __PID_H_
#pragma once

#include <map>
#include <memory>

#pragma warning(push, 4)

// Forward Declarations
//
class Namespace;
class PidNamespace;

//-----------------------------------------------------------------------------
// Pid
//
// Implements a reference-counted process identifier.  Each identifier is
// associated with one or more namespaces and can be different within each of
// those namespaces.  When accessing the underlying pid_t value, the specific
// namespace must be provided in order to acquire the correct one

class Pid
{
friend class PidNamespace;
public:

	// Destructor
	//
	~Pid();

	//-------------------------------------------------------------------------
	// Properties

	// Value
	//
	// Retrieves the pid_t value for a specific namespace
	__declspec(property(get=getValue)) uapi::pid_t Value[];
	uapi::pid_t getValue(const std::shared_ptr<Namespace>& ns) const;
	uapi::pid_t getValue(const std::shared_ptr<PidNamespace>& ns) const;

private:

	Pid(const Pid&)=delete;
	Pid& operator=(const Pid&)=delete;

	// pidmap_t
	//
	// Collection that holds all of the namespace->pid_t mappings
	using pidmap_t = std::map<std::shared_ptr<PidNamespace>, uapi::pid_t>;

	// Instance Constructor
	//
	Pid()=default;
	friend class std::_Ref_count_obj<Pid>;

	//-------------------------------------------------------------------------
	// Member Variables

	pidmap_t				m_pids;			// Namespace-specific pid_t values
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PID_H_
