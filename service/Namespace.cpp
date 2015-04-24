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
#include "Namespace.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Namespace Constructor (private)
//
// Arguments:
//
//	pidns		- PidNamespace instance to contain
//	utsns		- UtsNamespace instance to contain

Namespace::Namespace(const std::shared_ptr<PidNamespace>& pidns, const std::shared_ptr<UtsNamespace>& utsns) : 
	m_pidns(pidns), m_utsns(utsns)
{
}

//-----------------------------------------------------------------------------
// Namespace::Clone
//
// Creates a new namespace instance, either sharing or creating new instances
// of the individual namespace components (mounts, pids, etc)
//
// Arguments:
//
//	flags		- Namespace cloning options

std::shared_ptr<Namespace> Namespace::Clone(int flags)
{
	// Verify that a valid set of clone flags have been provided
	_ASSERTE((flags & ~CLONE_FLAGS) == 0);
	if(flags & ~CLONE_FLAGS) throw std::exception("todo");	// todo: exception

	// IPCNAMESPACE
	// auto ipcns = (flags & LINUX_CLONE_NEWIPC) ? 

	// MOUNTNAMESPACE
	// auto mountns = (flags & LINUX_CLONE_NEWNS) ? 

	// NETWORKNAMESPACE
	// auto networkns = (flags & LINUX_CLONE_NEWNET) ? 

	// PIDNAMESPACE
	auto pidns = (flags & LINUX_CLONE_NEWPID) ? PidNamespace::Create(m_pidns) : m_pidns;
	// ancestor

	// USERNAMESPACE
	// auto userns = (flags & LINUX_CLONE_NEWUSER) ? 

	// UTSNAMESPACE
	auto utsns = (flags & LINUX_CLONE_NEWUTS) ? UtsNamespace::Create(m_utsns) : m_utsns;

	// Construct the new Namespace with the selected individual components
	return std::make_shared<Namespace>(pidns, utsns);
}

//-----------------------------------------------------------------------------
// Namespace::Create (static)
//
// Constructs a new Namespace instance
//
// Arguments:
//
//	NONE

std::shared_ptr<Namespace> Namespace::Create(void)
{
	return std::make_shared<Namespace>(PidNamespace::Create(), UtsNamespace::Create());
}

//-----------------------------------------------------------------------------
// Namespace::getPid
//
// Accesses the contained PidNamespace instance

std::shared_ptr<PidNamespace> Namespace::getPid(void) const
{
	return m_pidns;
}

//-----------------------------------------------------------------------------
// Namespace::getUts
//
// Accesses the contained UtsNamespace instance

std::shared_ptr<UtsNamespace> Namespace::getUts(void) const
{
	return m_utsns;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
