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
	// this will need to check the current uid/gid against the contained values
}

//-----------------------------------------------------------------------------
// FilePermission::Narrow
//
// Narrows the permission set based on file access flags (O_RDONLY and so on)
//
// Arguments:
//
//	flags		- File access mask flags

void FilePermission::Narrow(int flags)
{
	// TODO: what to do about execute for RDONLY and RDWR ?

	switch(flags & LINUX_O_ACCMODE) {

		// O_RDONLY: Remove write permissions from the mode
		case LINUX_O_RDONLY: 
			m_mode &= ~(LINUX_S_IWUSR | LINUX_S_IWGRP | LINUX_S_IWOTH); 
			break;

		// O_WRONLY: Remove read and execute permissions from the mode
		case LINUX_O_WRONLY: 
			m_mode &= ~(LINUX_S_IRUSR | LINUX_S_IRGRP | LINUX_S_IROTH | LINUX_S_IXUSR | LINUX_S_IXGRP | LINUX_S_IXOTH); 
			break;

		// O_RDWR: Permissions remain unmodified
		case LINUX_O_RDWR:   
			break;
	}
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
