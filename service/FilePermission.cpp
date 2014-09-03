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

#include "stdafx.h"
#include "FilePermission.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// FilePermission Constructor
//
// Arguments:
//
//	mode		- Mode flags to assign to this FilePermission

FilePermission::FilePermission(uapi::mode_t mode) : m_mode(mode)
{
	// todo: need to get uid/gid from somewhere
	m_uid = m_gid = 0;
}

//-----------------------------------------------------------------------------
// FilePermission Constructor
//
// Arguments:
//
//	uid			- Owner UID to assign to this FilePermission
//	gid			- Group owner GID to assign to this FilePermission
//	mode		- Mode flags to assign to this FilePermission

FilePermission::FilePermission(uapi::uid_t uid, uapi::gid_t gid, uapi::mode_t mode) :
	m_uid(uid), m_gid(gid), m_mode(mode)
{
}

//-----------------------------------------------------------------------------
// FilePermission Copy Constructor

FilePermission::FilePermission(const FilePermission& rhs) : m_uid(rhs.m_uid),
	m_gid(rhs.m_gid), m_mode(rhs.m_mode)
{
}

//-----------------------------------------------------------------------------
// FilePermission:Demand
//
// Demands the caller has the specified access to the securable object
//
// Arguments:
//
//	access		- Access mask being demanded

void FilePermission::Demand(const FilePermission::Access& access)
{
	(access);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
