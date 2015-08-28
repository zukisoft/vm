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

#include "LinuxException.h"

#pragma warning(push, 4)

// FilePermission::Execute (static)
//
const FilePermission FilePermission::Execute{ FilePermission::EXECUTE };

// FilePermission::Read (static)
//
const FilePermission FilePermission::Read{ FilePermission::READ };

// FilePermission::Write (static)
//
const FilePermission FilePermission::Write{ FilePermission::WRITE };

//-----------------------------------------------------------------------------
// FilePermission Constructor (private)
//
// Arguments:
//
//	mask		- Access mask to be assigned to this instance

FilePermission::FilePermission(uint8_t mask) : m_mask{ mask }
{
}

//-----------------------------------------------------------------------------
// FilePermission bitwise or operator

FilePermission FilePermission::operator|(const FilePermission rhs) const
{
	return FilePermission{ static_cast<uint8_t>(m_mask | rhs.m_mask) };
}

//-----------------------------------------------------------------------------
// FilePermission::Check (static)
//
// Checks the specified access from permission components
//
// Arguments:
//
//	permission	- FilePermission representing requested access
//	uid			- File system object user id
//	gid			- File system object group id
//	mode		- File system object mode flags

bool FilePermission::Check(const FilePermission& permission, uapi::uid_t uid, uapi::gid_t gid, uapi::mode_t mode)
{
	// todo
	(permission);
	(uid);
	(gid);
	(mode);
	
	return true;
}

//-----------------------------------------------------------------------------
// FilePermission::Demand (static)
//
// Demands the specified access from permission components
//
// Arguments:
//
//	permission	- FilePermission representing requested access
//	uid			- File system object user id
//	gid			- File system object group id
//	mode		- File system object mode flags

void FilePermission::Demand(const FilePermission& permission, uapi::uid_t uid, uapi::gid_t gid, uapi::mode_t mode)
{
	// This is the same operation as Check(), it just throws an exception
	if(!Check(permission, uid, gid, mode)) throw LinuxException{ LINUX_EACCES };
}
	
//-----------------------------------------------------------------------------

#pragma warning(pop)
