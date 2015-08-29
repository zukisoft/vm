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

#ifndef __PROCESSFILESYSTEM_H_
#define __PROCESSFILESYSTEM_H_
#pragma once

#include <memory>
#include "FileSystem.h"

#pragma warning(push, 4)

// Forward Declarations
//
class Path;

//-----------------------------------------------------------------------------
// ProcessFileSystem
//
// Implements a container for process file system properties that can be duplicated
// or shared among multiple process instances

class ProcessFileSystem
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new file system information collection
	static std::shared_ptr<ProcessFileSystem> Create(const std::shared_ptr<FileSystem::Path>& rootdir, 
		const std::shared_ptr<FileSystem::Path>& workingdir, uapi::mode_t umask);

	// Duplicate
	//
	// Duplicates the collection into a new instance
	std::shared_ptr<ProcessFileSystem> Duplicate(void) const;

	// SetFileCreationModeMask
	//
	// Sets the permission mask used when creating new file system objects
	void SetFileCreationModeMask(uapi::mode_t umask);

	// SetRootDirectory
	//
	// Sets the root directory path
	void SetRootDirectory(const std::shared_ptr<FileSystem::Path>& path);

	// SetWorkingDirectory
	//
	// Sets the working directory path
	void SetWorkingDirectory(const std::shared_ptr<FileSystem::Path>& path);

	//-------------------------------------------------------------------------
	// Properties

	// FileCreationModeMask
	//
	// Gets the permission mask used when creating new file system objects
	__declspec(property(get=getFileCreationModeMask)) uapi::mode_t FileCreationModeMask;
	uapi::mode_t getFileCreationModeMask(void) const;

	// RootDirectory
	//
	// Gets a reference to the process root directory path
	__declspec(property(get=getRootDirectory)) std::shared_ptr<FileSystem::Path> RootDirectory;
	std::shared_ptr<FileSystem::Path> getRootDirectory(void) const;

	// WorkingDirectory
	//
	// Gets a reference to the process working directory path
	__declspec(property(get=getWorkingDirectory)) std::shared_ptr<FileSystem::Path> WorkingDirectory;
	std::shared_ptr<FileSystem::Path> getWorkingDirectory(void) const;

private:

	ProcessFileSystem(const ProcessFileSystem&)=delete;
	ProcessFileSystem& operator=(const ProcessFileSystem&)=delete;

	// lock_t
	//
	// Synchronization object type
	using lock_t = sync::reader_writer_lock;

	// Instance Constructor
	//
	ProcessFileSystem(const std::shared_ptr<FileSystem::Path>& rootdir, const std::shared_ptr<FileSystem::Path>& workingdir, uapi::mode_t umask);
	friend class std::_Ref_count_obj<ProcessFileSystem>;

	//---------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<FileSystem::Path>			m_rootdir;		// Root directory path
	std::shared_ptr<FileSystem::Path>			m_workingdir;	// Working directory path
	uapi::mode_t					m_umask;		// File creation mask
	mutable lock_t					m_lock;			// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESSFILESYSTEM_H_