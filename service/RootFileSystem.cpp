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
#include "RootFileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem::getNode (private)
//
// Accesses the topmost node referenced by this alias

FileSystem::NodePtr RootFileSystem::getNode(void) 
{ 
	std::lock_guard<std::mutex> critsec(m_lock);
	return m_mounted.empty() ? shared_from_this() : m_mounted.top();
}
	
//-----------------------------------------------------------------------------
// RootFileSystem::Mount (static)
//
// Mounts the root file system
//
// Arguments:
//
//	source		- Unused for root file system

FileSystemPtr RootFileSystem::Mount(const tchar_t* source)
{
	UNREFERENCED_PARAMETER(source);

	// Mounting the root file system is as simple as creating an instance of it
	return std::make_shared<RootFileSystem>();
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount (private)
//
// Mounts/binds a foreign node to this alias, obscuring the previous node
//
// Arguments:
//
//	node		- Foreign node to be mounted on this alias

void RootFileSystem::Mount(const FileSystem::NodePtr& node)
{
	_ASSERTE(node);

	// All that needs to be done for this file system is push the node
	std::lock_guard<std::mutex> critsec(m_lock);
	m_mounted.push(node);
}

//-----------------------------------------------------------------------------
// RootFileSystem::Unmount (private)
//
// Unmounts/unbinds a node from this alias, revealing the previously bound node
//
// Arguments:
//
//	NONE

void RootFileSystem::Unmount(void)
{
	// Pop the topmost node instance from the stack, if one even exists
	std::lock_guard<std::mutex> critsec(m_lock);
	if(!m_mounted.empty()) m_mounted.pop();
}
	
//-----------------------------------------------------------------------------
// RootFileSystem::ResolvePath (private, static)
//
// Resolves a FileSystem::Alias from a relative object path
//
// Arguments:
//
//	current		- Current Alias instance that was used to resolve this node
//	path		- Relative file system object path string

FileSystem::AliasPtr RootFileSystem::ResolvePath(const FileSystem::AliasPtr& current, const tchar_t* path)
{
	if(path == nullptr) throw LinuxException(LINUX_ENOENT);

	// The RootFileSystem node doesn't support any child objects; if the
	// name provided is an empty string, return ourselves otherwise fail
	if(*path == 0) return shared_from_this();
	throw LinuxException(LINUX_ENOENT);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
