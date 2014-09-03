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

#ifndef __FILEPERMISSION_H_
#define __FILEPERMISSION_H_
#pragma once

#include <linux/types.h>
#include <linux/stat.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// FilePermission
//
// Defines a simple Unix Permission set, which consists of a user id, group id
// and an access mode mask.  Operates similarly to .NET, invoke Demand with
// the permissions required for the operation and it will throw an exception
// if access is to be denied.

class FilePermission
{
public:

	// Constructors / Destructor
	//
	FilePermission(uapi::mode_t mode);
	FilePermission(uapi::uid_t uid, uapi::gid_t gid, uapi::mode_t mode);
	FilePermission(const FilePermission& rhs);
	~FilePermission()=default;

	// Enum Access
	//
	// Defines the specific access rights; matches Linux octal masks and
	// can be combined the same way "Execute | Read", for example
	enum class Access
	{
		Execute			= 01,
		Write			= 02,
		Read			= 04,
	};

	//-------------------------------------------------------------------------
	// Member Functions

	// Demand
	//
	// Demands the provided access of the underlying permission
	void Demand(const Access& access);

	//-------------------------------------------------------------------------
	// Properties

	// GroupOwner
	//
	// Gets the GID of the file group owner
	__declspec(property(get=getGroupOwner)) uapi::gid_t GroupOwner;
	uapi::gid_t getGroupOwner(void) const { return m_gid; }

	// Mode
	//
	// Gets the mode flags for the file permission
	__declspec(property(get=getMode)) uapi::mode_t Mode;
	uapi::mode_t getMode(void) const { return m_mode; }

	// Owner
	//
	// Gets the UID of the file owner
	__declspec(property(get=getOwner)) uapi::uid_t Owner;
	uapi::uid_t getOwner(void) const { return m_uid; }

private:

	FilePermission& operator=(const FilePermission&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	uapi::uid_t					m_uid;			// Permission user id
	uapi::gid_t					m_gid;			// Permission group id
	uapi::mode_t				m_mode;			// Permission mode mask
};

// ::FilePermission::Access Bitwise Operators
inline FilePermission::Access operator~(FilePermission::Access lhs) {
	return static_cast<FilePermission::Access>(~static_cast<uint32_t>(lhs));
}

inline FilePermission::Access operator&(FilePermission::Access lhs, FilePermission::Access rhs) {
	return static_cast<FilePermission::Access>(static_cast<uint32_t>(lhs) & (static_cast<uint32_t>(rhs)));
}

inline FilePermission::Access operator|(FilePermission::Access lhs, FilePermission::Access rhs) {
	return static_cast<FilePermission::Access>(static_cast<uint32_t>(lhs) | (static_cast<uint32_t>(rhs)));
}

inline FilePermission::Access operator^(FilePermission::Access lhs, FilePermission::Access rhs) {
	return static_cast<FilePermission::Access>(static_cast<uint32_t>(lhs) ^ (static_cast<uint32_t>(rhs)));
}

// ::FilePermission::Access Compound Assignment Operators
inline FilePermission::Access& operator&=(FilePermission::Access& lhs, FilePermission::Access rhs) 
{
	lhs = lhs & rhs;
	return lhs;
}

inline FilePermission::Access& operator|=(FilePermission::Access& lhs, FilePermission::Access rhs) 
{
	lhs = lhs | rhs;
	return lhs;
}

inline FilePermission::Access& operator^=(FilePermission::Access& lhs, FilePermission::Access rhs) 
{
	lhs = lhs ^ rhs;
	return lhs;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILEPERMISSION_H_
