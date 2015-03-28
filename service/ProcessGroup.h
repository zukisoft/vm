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

#ifndef __PROCESSGROUP_H_
#define __PROCESSGROUP_H_
#pragma once

#include <memory>
#include <linux/types.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ProcessGroup
//
// Implements a process group, which is essentially just a collection of process
// instances that can be signaled or used to indicate he target for a wait operation

class ProcessGroup
{
public:

	// Destructor
	//
	~ProcessGroup()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	//-------------------------------------------------------------------------
	// Properties

	// ProcessGroupId
	//
	// Gets the virtual process group identifier
	__declspec(property(get=getProcessGroupId)) uapi::pid_t ProcessGroupId;
	uapi::pid_t getProcessGroupId(void) const;

private:

	ProcessGroup(const ProcessGroup&)=delete;
	ProcessGroup& operator=(const ProcessGroup&)=delete;

	// Instance Constructor
	//
	ProcessGroup(uapi::pid_t pgid);
	friend class std::_Ref_count_obj<ProcessGroup>;

	//-------------------------------------------------------------------------
	// Member Variables

	const uapi::pid_t				m_pgid;			// Process group identifier
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESSGROUP_H_
