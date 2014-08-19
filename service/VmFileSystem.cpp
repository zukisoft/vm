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
#include "VmFileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// VmFileSystem Constructor
//
// Arguments:
//
//	rootfs		- Mounted FileSystem instance to serve as the root

VmFileSystem::VmFileSystem(const FileSystemPtr& rootfs) : m_rootfs(rootfs)
{
	_ASSERTE(rootfs);
}

//-----------------------------------------------------------------------------
// VmFileSystem::Create (static)
//
// Creates a new file system using the provided mount as the absolute root
//
// Arguments:
//
//	rootfs		- FileSystem instance to serve as the absolute root

std::unique_ptr<VmFileSystem> VmFileSystem::Create(const FileSystemPtr& rootfs)
{
	_ASSERTE(rootfs);
	if(rootfs == nullptr) throw LinuxException(LINUX_EINVAL);

	// todo: register the mount point?
	return std::make_unique<VmFileSystem>(rootfs);
}

//-----------------------------------------------------------------------------
// VmFileSystem::ResolvePath (private)
//
// Resolves an alias from an absolute file system path
//
// Arguments:
//
//	absolute	- Absolute path to the alias to resolve

FileSystem::AliasPtr VmFileSystem::ResolvePath(const tchar_t* absolute)
{
	if(absolute == nullptr) throw LinuxException(LINUX_ENOENT);

	// Remove leading slashes from the provided path and start at the root node
	while((*absolute) && (*absolute == _T('/'))) absolute++;
	return ResolvePath(m_rootfs->RootNode, absolute);
}

//-----------------------------------------------------------------------------
// VmFileSystem::ResolvePath (private)
//
// Resolves an alias from a path relative to an existing alias
//
// Arguments:
//
//	base		- Base alias instance to use for resolution
//	relative	- Relative path to resolve

FileSystem::AliasPtr VmFileSystem::ResolvePath(const FileSystem::AliasPtr& base, const tchar_t* relative)
{
	_ASSERTE(base);
	return ResolvePath(base->Node, relative);
}

//-----------------------------------------------------------------------------
// VmFileSystem::ResolvePath (private)
//
// Resolves an alias from a path relative to an existing node
//
// Arguments:
//
//	base		- Base node instance to use for resolution
//	relative	- Relative path to resolve

FileSystem::AliasPtr VmFileSystem::ResolvePath(const FileSystem::NodePtr& base, const tchar_t* relative)
{
	_ASSERTE(base);
	return base->ResolvePath(relative);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
