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
#include <Shlwapi.h>
#include <PathCch.h>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include "LinuxException.h"
#include "Win32Exception.h"
#include "FileSystem.h"
#include "MountOptions.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "pathcch.lib")

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// HostFileSystem
//
// todo: document
//
// todo: note that this does not support overmounting within the file system (yet?)
//
// need to decide on symbolic link behavior
//
// O_APPEND: The mode O_APPEND is supported and will be obeyed for file object
// writes, however there is no way to prevent a race condition with other
// processes that may be doing the same
//
// O_DIRECT: The mode O_DIRECT is supported and requires the caller to provide
// a properly aligned buffer, count and offset for reads and writes to file objects.
// The alignment is checked when the file handle is opened with O_DIRECT specified
// rather than once in the mountpoint metadata, as symbolic/hard links on the host
// could potentially redirect the path to another volume outside the mount point.
//
// Metadata cannot be synchronized separately from the file data, a call to 
// fdatasync() will result in the same operation as a call to fsync()

class HostFileSystem : public FileSystem
{
public:

	// Destructor
	//
	virtual ~HostFileSystem()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Mount
	//
	// Mounts the file system
	static FileSystemPtr Mount(const tchar_t* source, uint32_t flags, void* data);

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;

	// MountPoint
	//
	// Internal state and metadata about the mounted file system, all file
	// system objects will hold a reference to this object.  An operating
	// system handle to the mount point is also held to ensure that it 
	// can't be deleted as long as the file system has open objects
	class MountPoint
	{
	public:

		// Constructor / Destructor
		//
		MountPoint(HANDLE handle, uint32_t flags, const void* data);
		~MountPoint();

		//---------------------------------------------------------------------
		// Member Functions

		// ValidateHandle
		//
		// Verifies that a newly opened handle meets all mount criteria
		void ValidateHandle(HANDLE handle);

		//---------------------------------------------------------------------
		// Properties

		// BasePath
		//
		// The base host operating system path provided during construction
		__declspec(property(get=getBasePath)) const tchar_t* BasePath;
		const tchar_t* getBasePath(void) { return m_path.data(); }

		// Options
		//
		// Gets a reference to the contained MountOptions instance
		__declspec(property(get=getOptions)) MountOptions& Options;
		MountOptions& getOptions(void) { return m_options; }

	private:

		MountPoint(const MountPoint&)=delete;
		MountPoint& operator=(const MountPoint&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		HANDLE						m_handle;				// Mount point handle
		MountOptions				m_options;				// Mounting options
		std::vector<tchar_t>		m_path;					// Path to the mount point
		bool						m_verifypath = true;	// Flag to verify the path
	};

	// Node
	//
	// Specialization of FileSystem::Node for a host file system instance
	class Node : public FileSystem::Node, public FileSystem::Alias, public std::enable_shared_from_this<Node>
	{
	public:

		// Destructor
		//
		virtual ~Node();

		//---------------------------------------------------------------------
		// Member Functions

		// Construct (static)
		//
		// Constructs a new Node instance
		static std::shared_ptr<Node> Construct(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* path);
		static std::shared_ptr<Node> Construct(const std::shared_ptr<MountPoint>& mountpoint, std::vector<tchar_t>&& path);
		static std::shared_ptr<Node> Construct(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle);
		static std::shared_ptr<Node> Construct(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* path, HANDLE handle);
		static std::shared_ptr<Node> Construct(const std::shared_ptr<MountPoint>& mountpoint, std::vector<tchar_t>&& path, HANDLE handle);

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

		// Instance Constructor
		//
		Node(const std::shared_ptr<MountPoint>& mountpoint, std::vector<tchar_t>&& path, HANDLE handle);
		friend class std::_Ref_count_obj<Node>;

		//---------------------------------------------------------------------
		// FileSystem::Alias Implementation

		// Mount
		//
		// Mounts/binds a foreign node to this alias, obscuring the previous node
		virtual void Mount(const NodePtr&) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }

		// Unmount
		//
		// Unmounts/unbinds a node from this alias, revealing the previously bound node
		virtual void Unmount(void) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }

		// getName
		//
		// Gets the name associated with this alias
		virtual const tchar_t* getName(void) { return m_name; }

		// getNode
		//
		// Accesses the node pointed to by this alias
		virtual FileSystem::NodePtr getNode(void) { return shared_from_this(); }

		//---------------------------------------------------------------------
		// FileSystem::Node Implementation

		// CreateDirectory
		//
		// Creates a new directory node as a child of this node
		virtual void CreateDirectory(const tchar_t* name);

		// CreateFile
		//
		// Creates a new regular file node as a child of this node
		virtual FileSystem::HandlePtr CreateFile(const tchar_t* name, int flags);

		// CreateSymbolicLink
		//
		// Creates a new symbolic link as a child of this node
		virtual void CreateSymbolicLink(const tchar_t*, const tchar_t*) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
	
		// OpenHandle
		//
		// Creates a FileSystem::Handle instance for this node
		virtual FileSystem::HandlePtr OpenHandle(int flags);

		// ResolvePath
		//
		// Resolves a path for an alias that is a child of this alias
		virtual FileSystem::AliasPtr ResolvePath(const tchar_t* path);

		// getIndex
		//
		// Gets the node index
		virtual uint64_t getIndex(void) { return (static_cast<uint64_t>(m_info.nFileIndexHigh) << 32) | m_info.nFileIndexLow; }

		// getType
		//
		// Gets the node type
		virtual NodeType getType(void) { return m_type; }

		//---------------------------------------------------------------------
		// Private Member Functions

		// AppendToPath
		//
		// Appends additional path information to this node's path
		std::vector<tchar_t> AppendToPath(const tchar_t* more);

		// FlagsToAccess
		//
		// Converts linux fnctl flags to a Windows access mode
		static DWORD FlagsToAccess(int flags);
		
		//---------------------------------------------------------------------
		// Member Variables

		HANDLE							m_handle;		// Operating system handle
		BY_HANDLE_FILE_INFORMATION		m_info;			// Basic node information
		std::shared_ptr<MountPoint>		m_mountpoint;	// Reference to the mountpoint
		const tchar_t*					m_name;			// Name portion of the path
		std::vector<tchar_t>			m_path;			// Full path to the host node
		FileSystem::NodeType			m_type;			// Type of the node instance
	};

	// Handle
	//
	// Specialization of FileSystem::Handle for a host file system instance
	class Handle : public FileSystem::Handle
	{
	public:

		// Destructor
		//
		virtual ~Handle();

		//---------------------------------------------------------------------
		// Member Functions

		// Construct (static)
		//
		// Constructs a new Handle instance
		static std::shared_ptr<Handle> Construct(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<Node>& node, HANDLE handle, int flags)
		{
			return std::make_shared<Handle>(mountpoint, node, handle, flags);
		}

	private:

		Handle(const Handle&)=delete;
		Handle& operator=(const Handle&)=delete;

		// Instance Constructor
		//
		Handle(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<Node>& node, HANDLE handle, int flags);
		friend class std::_Ref_count_obj<Handle>;

		//---------------------------------------------------------------------
		// FileSystem::Handle Implementation

		// Read
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t Read(void* buffer, uapi::size_t count);

		// Sync
		//
		// Synchronizes all metadata and data associated with the file to storage
		virtual void Sync(void);

		// SyncData
		//
		// Synchronizes all data associated with the file to storage
		virtual void SyncData(void) { Sync(); }

		// Write
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t Write(const void* buffer, uapi::size_t count);

		//---------------------------------------------------------------------
		// Private Member Functions

		// ValidateAlignment
		//
		// Verifies that the data used when O_DIRECT is used is valid
		void ValidateAlignment(const void* buffer, uapi::size_t count);

		//---------------------------------------------------------------------
		// Member Variables

		uint32_t					m_alignment;		// File alignment info
		int							m_flags;			// File control flags
		HANDLE						m_handle;			// Operating system handle
		std::shared_ptr<MountPoint>	m_mountpoint;		// Reference to the mountpoint
		std::shared_ptr<Node>		m_node;				// Reference to the node instance
	};

	// Instance Constructor
	//
	HostFileSystem(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<Node>& root);
	friend class std::_Ref_count_obj<HostFileSystem>;

	//-------------------------------------------------------------------------
	// FileSystem Implementation

	// getRoot
	//
	// Accesses the root alias for the file system
	virtual AliasPtr getRoot(void) { return m_root; }

	//-------------------------------------------------------------------------
	// Private Member Functions

	// MapException (static)
	//
	// Returns a mapped LinuxException based on a Win32 error code
	static LinuxException MapException(void) { return MapException(GetLastError()); }
	static LinuxException MapException(DWORD code);

	//-------------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<MountPoint>	m_mountpoint;	// Contained mountpoint
	std::shared_ptr<Node>		m_root;			// Mounted root object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOSTFILESYSTEM_H_