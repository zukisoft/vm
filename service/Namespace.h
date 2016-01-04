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

#ifndef __NAMESPACE_H_
#define __NAMESPACE_H_
#pragma once

#include <memory>
#include "MountNamespace.h"
#include "PidNamespace.h"
#include "UtsNamespace.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// Namespace
//
// Wraps certain global system resources into an abstraction that makes it appear 
// to any processes within the namespace that they have their own isolated instances 
// of these resources:
//
//	Ipc			- Isolates System V IPC and posix message queues
//	Mount		- Isolates file system mount points
//	Network		- Isolates network devices, ports, stacks, etc.
//	Pid			- Isolates process identifiers
//	User		- Isolates user and group identifiers
//	Uts			- Isolates host and domain name strings

class Namespace
{
public:

	// Destructor
	//
	~Namespace()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Clone
	//
	// Creates a new Namespace instance based on this namespace
	std::shared_ptr<Namespace> Clone(int flags);

	// Create (static)
	//
	// Creates a new Namespace instance
	static std::shared_ptr<Namespace> Create(void);

	//-------------------------------------------------------------------------
	// Properties

	// Mounts
	//
	// Accesses the contained MountNamespace instance
	__declspec(property(get=getMounts)) std::shared_ptr<MountNamespace> Mounts;
	std::shared_ptr<MountNamespace> getMounts(void) const;

	// Pids
	//
	// Accesses the contained PidNamespace instance
	__declspec(property(get=getPids)) std::shared_ptr<PidNamespace> Pids;
	std::shared_ptr<PidNamespace> getPids(void) const;

	// UtsNames
	//
	// Acceses the contained UtsNamespace instance
	__declspec(property(get=getUts)) std::shared_ptr<UtsNamespace> UtsNames;
	std::shared_ptr<UtsNamespace> getUtsNames(void) const;

private:

	Namespace(const Namespace&)=delete;
	Namespace& operator=(const Namespace&)=delete;

	// CLONE_FLAGS
	//
	// Defines the possible CLONE_XXX flags that can be passed into Clone()
	const int CLONE_FLAGS = (LINUX_CLONE_NEWIPC | LINUX_CLONE_NEWNET | LINUX_CLONE_NEWNS | LINUX_CLONE_NEWPID | LINUX_CLONE_NEWUSER | LINUX_CLONE_NEWUTS);

	// Instance Constructor
	//
	Namespace(const std::shared_ptr<MountNamespace>& mountns, const std::shared_ptr<PidNamespace>& pidns, const std::shared_ptr<UtsNamespace>& utsns);
	friend class std::_Ref_count_obj<Namespace>;

	//-------------------------------------------------------------------------
	// Member Variables

	const std::shared_ptr<MountNamespace>	m_mountns;		// MountNamespace
	const std::shared_ptr<PidNamespace>		m_pidns;		// PidNamespace
	const std::shared_ptr<UtsNamespace>		m_utsns;		// UtsNamespace
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NAMESPACE_H_
