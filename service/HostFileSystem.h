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
#include <PathCch.h>
#include <Shlwapi.h>
#include <linux/errno.h>
#include "Exception.h"
#include "FileSystem.h"
#include "HeapBuffer.h"
#include "LinuxException.h"
#include "MountOptions.h"
#include "Win32Exception.h"

#pragma comment(lib, "pathcch.lib")
#pragma comment(lib, "shlwapi.lib")

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

	// Mount
	//
	// Mounts the file system
	static FileSystemPtr Mount(const uapi::char_t* source, uint32_t flags, void* data);

	// FileSystem Implementation
	//
	virtual FileSystem::AliasPtr getRoot(void) { return m_root; }

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;

	// Forward Declarations
	//
	class Alias;
	class DirectoryNode;
	class FileNode;
	class MountPoint;

	// Instance Constructor
	//
	HostFileSystem(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<Alias>& root) : m_mountpoint(mountpoint), m_root(root) {}
	friend class std::_Ref_count_obj<HostFileSystem>;

	// Member Variables
	//
	std::shared_ptr<MountPoint>		m_mountpoint;	// Mount metadata/state
	std::shared_ptr<Alias>			m_root;			// Root alias instance

	//-------------------------------------------------------------------------
	// HostFileSystem::Alias
	//
	class Alias : public FileSystem::Alias
	{
	public:

		virtual ~Alias()=default;

		// Construct (static)
		//
		// Constructs a new Alias instance
		static std::shared_ptr<Alias> Construct(const uapi::char_t* name, const FileSystem::NodePtr& node) { return Construct(name, nullptr, node); }
		static std::shared_ptr<Alias> Construct(const uapi::char_t* name, const FileSystem::AliasPtr& parent, const FileSystem::NodePtr& node);

		// FileSystem::Alias Implementation
		//
		virtual void					Mount(const FileSystem::NodePtr&) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
		virtual void					Unmount(void) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
		virtual const uapi::char_t*		getName(void) { return m_name.c_str(); }
		virtual FileSystem::NodePtr		getNode(void) { return m_node; }
		virtual FileSystem::AliasPtr	getParent(void);

	private:

		Alias(const Alias&)=delete;
		Alias& operator=(const Alias&)=delete;

		// Instance Constructor
		//
		Alias(const uapi::char_t* name, const FileSystem::AliasPtr& parent, const FileSystem::NodePtr& node) : m_name(name), m_parent(parent), m_node(node) {}
		friend class std::_Ref_count_obj<Alias>;

		// Member Variables
		//
		std::string							m_name;		// Alias name
		FileSystem::NodePtr					m_node;		// Referenced node
		std::weak_ptr<FileSystem::Alias>	m_parent;	// Parent alias
	};

	//-------------------------------------------------------------------------
	// HostFileSystem::DirectoryNode
	//
	class DirectoryNode : public FileSystem::Directory
	{
	public:

		virtual ~DirectoryNode();

		// FromHandle
		//
		// Constructs a new DirectoryNode instance from an existing handle
		static std::shared_ptr<DirectoryNode> FromHandle(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle);

		// FromPath
		//
		// Constructs a new DirectoryNode instance from a host file system path
		static std::shared_ptr<DirectoryNode> FromPath(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* path);

		// FileSystem::Node Implementation
		//
		virtual FileSystem::HandlePtr	Open(int flags);
		virtual FileSystem::AliasPtr	Resolve(const AliasPtr&, const AliasPtr& current, const uapi::char_t* path, int flags, int*);
		virtual uint64_t				getIndex(void);
		virtual FileSystem::NodeType	getType(void);

		// FileSystem::Directory Implementation
		//
		virtual void					CreateDirectory(const FileSystem::AliasPtr& parent, const uapi::char_t* name);
		virtual FileSystem::HandlePtr	CreateFile(const FileSystem::AliasPtr& parent, const uapi::char_t* name, int flags);
		virtual void					CreateSymbolicLink(const FileSystem::AliasPtr&, const uapi::char_t*, const uapi::char_t*);
		virtual void					RemoveNode(const uapi::char_t* name);

	private:

		DirectoryNode(const DirectoryNode&)=delete;
		DirectoryNode& operator=(const DirectoryNode&)=delete;

		// Instance Constructor
		//
		DirectoryNode(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle);
		friend class std::_Ref_count_obj<DirectoryNode>;

		// Member Variables
		//
		std::shared_ptr<MountPoint>		m_mountpoint;		// Mounted file system metadata
		HANDLE							m_handle;			// Query-only object handle
		HeapBuffer<tchar_t>				m_hostpath;			// Host operating system path

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
	class FileNode : public FileSystem::File
	{
	public:

		virtual ~FileNode();

		// FromHandle
		//
		// Constructs a new DirectoryNode instance from an existing handle
		static std::shared_ptr<FileNode> FromHandle(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle);

		// FromPath
		//
		// Constructs a new DirectoryNode instance from a host file system path
		static std::shared_ptr<FileNode> FromPath(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* path);

		// FileSystem::Node Implementation
		//
		virtual FileSystem::HandlePtr	Open(int flags);
		virtual FileSystem::AliasPtr	Resolve(const AliasPtr&, const AliasPtr& current, const uapi::char_t* path, int flags, int*);
		virtual uint64_t				getIndex(void);
		virtual FileSystem::NodeType	getType(void);

		// FileSystem::File Implementation
		//
		virtual HandlePtr				OpenExec(int flags);

	private:

		FileNode(const FileNode&)=delete;
		FileNode& operator=(const FileNode&)=delete;

		// Instance Constructor
		//
		FileNode(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle);
		friend class std::_Ref_count_obj<FileNode>;

		// Member Variables
		//
		std::shared_ptr<MountPoint>		m_mountpoint;		// Mounted file system metadata
		HANDLE							m_handle;			// Query-only object handle

		// FileNode::Handle
		//
		// Standard I/O handle opened against a FileNode instance
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
		// Specialized execute-only handle opened against a FileNode instance
		class ExecHandle : public FileSystem::Handle
		{
		public:

			virtual ~ExecHandle();

			// FileSystem::Handle Implementation
			//
			virtual uapi::size_t	Read(void* buffer, uapi::size_t count);
			virtual uapi::loff_t	Seek(uapi::loff_t offset, int whence);
			virtual void			Sync(void);
			virtual void			SyncData(void);
			virtual uapi::size_t	Write(const void*, uapi::size_t);

		private:

			ExecHandle(const ExecHandle&)=delete;
			ExecHandle& operator=(const ExecHandle&)=delete;

			// Instance Constructor
			//
			ExecHandle(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle);
			friend class std::_Ref_count_obj<ExecHandle>;

			// Member Variables
			//
			std::shared_ptr<MountPoint>		m_mountpoint;		// Mounted file system metadata
			HANDLE							m_handle;			// Read/execute object handle
		};
	};

	//-------------------------------------------------------------------------
	// HostFileSystem::MountPoint
	//
	class MountPoint
	{
	public:

		// Constructor / Destructor
		//
		MountPoint(HANDLE handle, uint32_t flags, const void* data);
		~MountPoint();

		// HostPath
		//
		// Gets the host file system path that was mounted
		__declspec(property(get=getHostPath)) const tchar_t* HostPath;
		const tchar_t* getHostPath(void) { return m_hostpath; }

		// Options
		//
		// Gets a reference to the contained MountOptions instance
		__declspec(property(get=getOptions)) MountOptions& Options;
		MountOptions& getOptions(void) { return m_options; }

	private:

		MountPoint(const MountPoint&)=delete;
		MountPoint& operator=(const MountPoint&)=delete;

		// Member Variables
		//
		HANDLE					m_handle;			// Mounted directory handle
		MountOptions			m_options;			// Standard mounting options
		HeapBuffer<tchar_t>		m_hostpath;			// The mounted host path
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