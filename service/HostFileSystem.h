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

#ifndef __HOSTFILESYSTEM_H_
#define __HOSTFILESYSTEM_H_
#pragma once

#include <memory>
#include "FileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// HostFileSystem
//
// HostFileSystem mounts a directory on the host system and makes it available
// as part of the virtual file system
//
// NOTES:
//
//	Symbolic Links are not directly supported (todo: words)
//	Overmounting is not supported (todo: words - implies no devices, sockets, etc)
//
// FILESYSTEM OBJECTS:
//
//	HostFileSystem::Alias					- File system object name
//	HostFileSystem::DirectoryNode			- Directory object
//	HostFileSystem::DirectoryNode::Handle	- Directory object handle
//	HostFileSystem::FileNode				- File object
//	HostFileSystem::FileNode::ExecHandle	- Execute-only file object handle
//	HostFileSystem::FileNode::Handle		- Standard file object handle
//	HostFileSystem::MountPoint				- Mount point state and metadata
//	HostFileSystem::PathHandle				- Path-only object handle
//
// CUSTOM MOUNT OPTIONS:
//

class HostFileSystem : public FileSystem
{
public:

	virtual ~HostFileSystem()=default;

	// FileSystem Implementation
	//
	virtual FileSystem::AliasPtr getRoot(void) { return nullptr; }

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;

	// Member Variables
	//

	//-------------------------------------------------------------------------
	// HostFileSystem::Alias
	//
	class Alias
	{
	public:

		virtual ~Alias()=default;

	private:

		Alias(const Alias&)=delete;
		Alias& operator=(const Alias&)=delete;
	};

	//-------------------------------------------------------------------------
	// HostFileSystem::DirectoryNode
	//
	class DirectoryNode
	{
	public:

		virtual ~DirectoryNode()=default;

	private:

		DirectoryNode(const DirectoryNode&)=delete;
		DirectoryNode& operator=(const DirectoryNode&)=delete;

		// DirectoryNode::Handle
		//
		class Handle
		{
		public:

			virtual ~Handle()=default;

		private:

			Handle(const Handle&)=delete;
			Handle& operator=(const Handle&)=delete;
		};
	};

	//-------------------------------------------------------------------------
	// HostFileSystem::FileNode
	//
	class FileNode
	{
	public:

		virtual ~FileNode()=default;

	private:

		FileNode(const FileNode&)=delete;
		FileNode& operator=(const FileNode&)=delete;

		// FileNode::Handle
		//
		class Handle
		{
		public:

			virtual ~Handle()=default;

		private:

			Handle(const Handle&)=delete;
			Handle& operator=(const Handle&)=delete;
		};

		// FileNode::ExecHandle
		//
		class ExecHandle
		{
		public:

			virtual ~ExecHandle()=default;

		private:

			ExecHandle(const ExecHandle&)=delete;
			ExecHandle& operator=(const ExecHandle&)=delete;
		};
	};

	//-------------------------------------------------------------------------
	// HostFileSystem::MountPoint
	//
	class MountPoint
	{
	public:

		virtual ~MountPoint()=default;

	private:

		MountPoint(const MountPoint&)=delete;
		MountPoint& operator=(const MountPoint&)=delete;
	};

	//-------------------------------------------------------------------------
	// HostFileSystem::PathHandle
	//
	class PathHandle
	{
	public:

		virtual ~PathHandle()=default;

	private:

		PathHandle(const PathHandle&)=delete;
		PathHandle& operator=(const PathHandle&)=delete;
	};
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOSTFILESYSTEM_H_