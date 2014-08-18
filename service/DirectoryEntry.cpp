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
#include "DirectoryEntry.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// DirectoryEntry Constructor (private)
//
// Arguments:
//
//	name		- Name to assign to this directory entry
//	parent		- Parent DirectoryEntry or nullptr if this is a root node
//	node		- Node to attach or nullptr to create a detached entry

DirectoryEntry::DirectoryEntry(const tchar_t* name, const DirectoryEntryPtr& parent, const FileSystem::NodePtr& node)
{
	// test name - throw if no good; root should use "/" I think

	m_name = name;
	if(parent) m_parent = parent;
	if(node) m_nodes.push(node);
}

DirectoryEntryPtr DirectoryEntry::Create(const tchar_t* name)
{
	return Create(name, nullptr, nullptr);
}

DirectoryEntryPtr DirectoryEntry::Create(const tchar_t* name, const DirectoryEntryPtr& parent)
{
	return Create(name, parent, nullptr);
}

DirectoryEntryPtr DirectoryEntry::Create(const tchar_t* name, const DirectoryEntryPtr& parent, const FileSystem::NodePtr& node)
{
	return std::make_shared<DirectoryEntry>(name, parent, node);
}


DirectoryEntryPtr DirectoryEntry::CreateDirectory(const tchar_t* name, uapi::mode_t mode)
{
	std::lock_guard<std::recursive_mutex> critsec(m_lock);

	// must be attached
	//if(m_nodes.empty()) throw something;

	DirectoryEntryPtr child = Create(name, shared_from_this());
	child->PushNode(m_nodes.top()->CreateDirectory(name, mode));

	// add to collection
	m_children.push_back(child);
	return child;
}

DirectoryEntryPtr DirectoryEntry::CreateSymbolicLink(const tchar_t* name, const tchar_t* target)
{
	std::lock_guard<std::recursive_mutex> critsec(m_lock);

	// must be attached
	//if(m_nodes.empty()) throw something;

	DirectoryEntryPtr child = Create(name, shared_from_this());
	child->PushNode(m_nodes.top()->CreateSymbolicLink(name, target));

	// add to collection
	m_children.push_back(child);
	return child;
}

void DirectoryEntry::PushNode(const FileSystem::NodePtr& node)
{
	// check node for nullptr
	std::lock_guard<std::recursive_mutex> critsec(m_lock);
	m_nodes.push(node);
}

void DirectoryEntry::PopNode(void)
{
	std::lock_guard<std::recursive_mutex> critsec(m_lock);

	if(m_nodes.empty()) throw LinuxException(LINUX_EINVAL);
	m_nodes.pop();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
