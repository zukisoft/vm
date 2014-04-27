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

#ifndef __VFSNODE_H_
#define __VFSNODE_H_
#pragma once

#include <queue>
#include "tstring.h"
#include "AutoCriticalSection.h"
#include "CriticalSection.h"
#include "Exception.h"
#include "Win32Exception.h"

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VfsNodePtr
//
// Typedef of std::shared_ptr<VfsNode>

class VfsNode;
typedef std::shared_ptr<VfsNode> VfsNodePtr;

//-----------------------------------------------------------------------------
// VfsNode
//
// Base class for all virtual file system nodes

class __declspec(novtable) VfsNode
{
public:

	// Destructor
	//
	virtual ~VfsNode();

	//-------------------------------------------------------------------------
	// Properties

	// GroupId
	//
	// Gets/sets the node owner group id
	__declspec(property(get=getGroupId, put=putGroupId)) gid_t GroupId;
	gid_t getGroupId(void) const { return m_gid; }
	void putGroupId(gid_t value);

	// Index
	//
	// Gets the node index
	__declspec(property(get=getIndex)) int32_t Index;
	int32_t getIndex(void) const { return m_index; }

	// Mode
	//
	// Gets/sets the mode flags for this node
	__declspec(property(get=getMode, put=putMode)) mode_t Mode;
	uint32_t getMode(void) const { return m_mode; }
	void putMode(mode_t value);

	// UserId
	//
	// Gets/sets the node owner user id
	__declspec(property(get=getUserId, put=putUserId)) uid_t UserId;
	uid_t getUserId(void) const { return m_uid; }
	void putUserId(uid_t value);

protected:

	// Instance Constructors
	//
	VfsNode(mode_t mode) : m_index(AllocateIndex()), m_mode(mode) { /*TODO: check index >= 0 */ }
	VfsNode(mode_t mode, uid_t uid, gid_t gid) : m_index(AllocateIndex()), m_mode(mode), m_uid(uid), m_gid(gid) {}

	//-------------------------------------------------------------------------
	// Protected Member Functions

	// GenerateTemporaryFileName
	//
	// Generates a UUID-based name to represent a temporary file
	static std::tstring GenerateTemporaryFileName(void);

private:

	VfsNode(const VfsNode&)=delete;
	VfsNode& operator=(const VfsNode&)=delete;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AllocateIndex
	//
	// Allocates a unique node index from the index pool
	static int32_t AllocateIndex(void);

	// ReleaseIndex
	//
	// Releases a node index back into the index pool
	static void ReleaseIndex(int32_t index);

	//-------------------------------------------------------------------------
	// Member Variables

	const int32_t				m_index;			// Node index
	mode_t						m_mode;				// Node mode flags
	uid_t						m_uid = 0;			// Node owner
	gid_t						m_gid = 0;			// Node owner

	static int32_t				s_next;				// Next sequential index
	static std::queue<int32_t>	s_spent;			// Spent indexes
	static CriticalSection		s_cs;				// Index allocator lock

	static std::tstring			s_tempdir;			// Temporary node storage
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VFSNODE_H_
