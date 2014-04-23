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
#include "VfsNode.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

// VfsNode::s_next (static)
//
// Next sequential node index
int32_t VfsNode::s_next = 1;

// VfsNode::s_spent (static)
//
// Queue of spent and available node indexes
std::queue<int32_t> VfsNode::s_spent;

// VfsNode::s_cs (static)
//
// Critical Section for access to the index allocator
CriticalSection VfsNode::s_cs;

//-----------------------------------------------------------------------------
// VfsNode Destructor

VfsNode::~VfsNode()
{
	ReleaseIndex(m_index);				// Release the allocated index
}

//-----------------------------------------------------------------------------
// VfsNode::AddRef
//
// Increments the object reference counter
//
// Arguments:
//
//	NONE

VfsNode* VfsNode::AddRef(void) 
{
	InterlockedIncrement(&m_ref);
	return this;
}

//-----------------------------------------------------------------------------
// VfsNode::AllocateIndex (static)
//
// Allocates a new node index, returns -1 if there are no more available
//
// Arguments:
//
//	NONE

int32_t VfsNode::AllocateIndex(void)
{
	uint32_t				index;				// Index to return

	AutoCriticalSection cs(s_cs);

	// Ressurect a spent index when one is available, otherwise generate
	// a sequentially new index for this node
	if(!s_spent.empty()) { index = s_spent.front(); s_spent.pop(); }
	else index = (s_next == INT32_MAX) ? -1 : s_next++;

	return index;
}

//-----------------------------------------------------------------------------
// VfsNode::putGroupId

void VfsNode::putGroupId(gid_t value)
{
	UNREFERENCED_PARAMETER(value);
	throw Exception(E_NOTIMPL);
}

//-----------------------------------------------------------------------------
// VfsNode::putMode

void VfsNode::putMode(mode_t value)
{
	UNREFERENCED_PARAMETER(value);
	throw Exception(E_NOTIMPL);
}

//-----------------------------------------------------------------------------
// VfsNode::putUserId

void VfsNode::putUserId(uid_t value)
{
	UNREFERENCED_PARAMETER(value);
	throw Exception(E_NOTIMPL);
}

//-----------------------------------------------------------------------------
// VfsNode::Release
//
// Decrements the object reference counter and self-deletes when it reaches 0
//
// Arguments:
//
//	NONE

void VfsNode::Release(void)
{
	if(InterlockedDecrement(&m_ref) == 0) delete this;
}

//-----------------------------------------------------------------------------
// VfsNode::ReleaseIndex (static)
//
// Releases a previously allocated node index
//
// Arguments:
//
//	index		- Spent node index

void VfsNode::ReleaseIndex(int32_t index)
{
	AutoCriticalSection cs(s_cs);
	s_spent.push(index);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
