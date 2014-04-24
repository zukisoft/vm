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

// RPC_TSTR
//
// Macro indicating the proper string width from RPC function calls
#pragma push_macro("RPC_TSTR")
#undef RPC_TSTR
#ifdef _UNICODE
#define RPC_TSTR	RPC_WSTR
#else
#define RPC_TSTR	RPC_CSTR
#endif

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

// VfsNode::s_tempdir (static)
//
// Location where all the temporary node storage files will be placed
std::tstring VfsNode::s_tempdir = []() -> std::tstring {

	tchar_t	temppath[MAX_PATH + 1];			// Temporary path buffer

	// Get the temporary path for the current process identity; return
	// an empty string on failure, don't want to throw exceptions here
	if(GetTempPath(MAX_PATH + 1, temppath) == 0) return std::tstring();
	else return std::tstring(temppath);
}();

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
// VfsNode::GenerateTemporaryFileName (static, protected)
//
// Generates a unique file name for an underlying OS file
//
// Arguments:
//
//	NONE

std::tstring VfsNode::GenerateTemporaryFileName(void)
{
	UUID			uuid;				// Folder UUID 
	RPC_TSTR		uuidstr;			// Folder UUID as a string
	RPC_STATUS		rpcstatus;			// Result from RPC function call

	// Create a UUID with the RPC runtime rather than the COM runtime
	rpcstatus = UuidCreate(&uuid);
	if((rpcstatus != RPC_S_OK) && (rpcstatus != RPC_S_UUID_LOCAL_ONLY)) throw Win32Exception(rpcstatus);

	// Convert the UUID into a string
	rpcstatus = UuidToString(&uuid, &uuidstr);
	if(rpcstatus != RPC_S_OK) throw Win32Exception(rpcstatus);

	// Convert the string into a std::tstring and release the RPC buffer
	std::tstring filename(reinterpret_cast<tchar_t*>(uuidstr));
	RpcStringFree(&uuidstr);

	// Append the generated file name to the temporary directory name
	return s_tempdir + filename;
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

#pragma pop_macro("RPC_TSTR")

#pragma warning(pop)
