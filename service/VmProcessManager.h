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

#ifndef __VMPROCESSMANAGER_H_
#define __VMPROCESSMANAGER_H_
#pragma once

#include <concurrent_unordered_map.h>
#include <memory>
#include "Process.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// VmProcessManager
//
// Manages all processes hosted by a virtual machine instance

class VmProcessManager
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

private:

	VmProcessManager(const VmProcessManager&)=delete;
	VmProcessManager& operator=(const VmProcessManager&)=delete;

	//-------------------------------------------------------------------------
	// Private Member Functions

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// host_pid_t
	//
	// Typedef for the host system process identifier
	using host_pid_t = DWORD;

	// process_ptr_t
	//
	// Typedef for an std::shared_ptr<Process> instance
	using process_ptr_t = std::shared_ptr<Process>;

	// process_map_t
	//
	// Typedef for a concurrent map<> of hosted processes
	using process_map_t = Concurrency::concurrent_unordered_map<host_pid_t, process_ptr_t>;

	//-------------------------------------------------------------------------
	// Member Variables

	process_map_t			m_processes;	// Collection of hosted processes
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMPROCESSMANAGER_H_
