//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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
#include "ProcessFileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ProcessFileSystem Constructor (private)
//
// Arguments:
//
//	rootdir		- Reference to the root directory path
//	workingdir	- Reference to the working directory path
//	umask		- Default file permission mask

ProcessFileSystem::ProcessFileSystem(const std::shared_ptr<FileSystem::Path>& rootdir, const std::shared_ptr<FileSystem::Path>& workingdir, uapi::mode_t umask)
	: m_rootdir(rootdir), m_workingdir(workingdir), m_umask(umask)
{
	_ASSERTE(rootdir);
	_ASSERTE(workingdir);
}

//-----------------------------------------------------------------------------
// ProcessFileSystem::Create (static)
//
// Creates a new file system information collection
//
// Arguments:
//
//	rootdir		- Reference to the root directory path
//	workingdir	- Reference to the working directory path
//	umask		- Default file permission mask

std::shared_ptr<ProcessFileSystem> ProcessFileSystem::Create(const std::shared_ptr<FileSystem::Path>& rootdir, const std::shared_ptr<FileSystem::Path>& workingdir, uapi::mode_t umask)
{
	return std::make_shared<ProcessFileSystem>(rootdir, workingdir, umask & LINUX_S_IRWXUGO);
}

//-----------------------------------------------------------------------------
// ProcessFileSystem::Duplicate
//
// Duplicates the collection into a new instance
//
// Arguments:
//
//	NONE

std::shared_ptr<ProcessFileSystem> ProcessFileSystem::Duplicate(void) const
{
	lock_t::scoped_lock_read reader(m_lock);
	return Create(m_rootdir, m_workingdir, m_umask);
}

//-----------------------------------------------------------------------------
// ProcessFileSystem::getFileCreationModeMask
//
// Gets the permission mask used when creating new file system objects

uapi::mode_t ProcessFileSystem::getFileCreationModeMask(void) const
{
	lock_t::scoped_lock_read reader(m_lock);
	return m_umask;
}

//-----------------------------------------------------------------------------
// ProcessFileSystem::getRootDirectory
//
// Gets a reference to the process root directory path 

std::shared_ptr<FileSystem::Path> ProcessFileSystem::getRootDirectory(void) const
{
	lock_t::scoped_lock_read reader(m_lock);
	return m_rootdir;
}

//-----------------------------------------------------------------------------
// ProcessFileSystem::SetFileCreationModeMask
//
// Sets the permission mask used when creating new file system objects
//
// Arguments:
//
//	umask		- New default file permission mask

void ProcessFileSystem::SetFileCreationModeMask(uapi::mode_t umask)
{
	lock_t::scoped_lock_write writer(m_lock);
	m_umask = umask & LINUX_S_IRWXUGO;		// == umask & 0777
}

//-----------------------------------------------------------------------------
// ProcessFileSystem::SetRootDirectory
//
// Sets the root directory path
//
// Arguments:
//
//	path		- Path instance representing the root directory

void ProcessFileSystem::SetRootDirectory(const std::shared_ptr<FileSystem::Path>& path)
{
	lock_t::scoped_lock_write writer(m_lock);
	m_rootdir = path;
}

//-----------------------------------------------------------------------------
// ProcessFileSystem::SetWorkingDirectory
//
// Sets the working directory path
//
// Arguments:
//
//	path		- Path instance representing the working directory

void ProcessFileSystem::SetWorkingDirectory(const std::shared_ptr<FileSystem::Path>& path)
{
	lock_t::scoped_lock_write writer(m_lock);
	m_workingdir = path;
}

//-----------------------------------------------------------------------------
// ProcessFileSystem::getWorkingDirectory
//
// Gets a reference to the process working directory path 

std::shared_ptr<FileSystem::Path> ProcessFileSystem::getWorkingDirectory(void) const
{
	lock_t::scoped_lock_read reader(m_lock);
	return m_workingdir;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
