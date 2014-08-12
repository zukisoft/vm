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
// RootFileSystem::Mount (static)
//
// Mounts the root file system
//
// Arguments:
//
//	device		- Unused for root file system

FileSystem::AliasPtr RootFileSystem::Mount(const tchar_t* device)
{
	UNREFERENCED_PARAMETER(device);

	// Mounting the root file system just creates an instance of it
	// and returns the FileSystem::Alias interface pointer
	return std::make_shared<RootFileSystem>();
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount (private)
//
// Mounts a node in this alias instance
//
// Arguments:
//
//	node		- Node to be mounted in this alias

void RootFileSystem::Mount(const NodePtr& node)
{
	_ASSERTE(node);
	if(node == nullptr) throw LinuxException(LINUX_ENOENT);

	// Push the new mounted node onto the stack
	std::lock_guard<std::mutex> critsec(m_lock);
	m_nodes.push(node);
}

//-----------------------------------------------------------------------------
// RootFileSystem::getMountPoint (private)
//
// Determines if this alias is serving as a mount point

bool RootFileSystem::getMountPoint(void)
{
	// If any nodes have been mounted, this alias is a mount point
	std::lock_guard<std::mutex> critsec(m_lock);
	return !m_nodes.empty();
}

//-----------------------------------------------------------------------------
// RootFileSystem::getNode (private)
//
// Retrieves a pointer to the Node attached to this Alias instance

FileSystem::NodePtr RootFileSystem::getNode(void)
{
	// If there are no over-mounted nodes, return this root instance
	std::lock_guard<std::mutex> critsec(m_lock);
	return (m_nodes.empty()) ? shared_from_this() : m_nodes.top();
}

//-----------------------------------------------------------------------------
// RootFileSystem::Unmount (private)
//
// Removes the topmost mounted node from this alias instance
//
// Arguments:
//
//	NONE

void RootFileSystem::Unmount(void)
{
	// Throw EINVAL if this alias is not serving as a mount point
	std::lock_guard<std::mutex> critsec(m_lock);
	if(m_nodes.empty()) throw LinuxException(LINUX_EINVAL);
	m_nodes.pop();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
