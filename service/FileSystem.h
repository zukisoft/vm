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

#ifndef __FILESYSTEM_H_
#define __FILESYSTEM_H_
#pragma once

#include <memory>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <linux/statfs.h>
#include <linux/types.h>

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

// Forward Declarations
//
class Namespace;

//-----------------------------------------------------------------------------
// FileSystem
//
// FileSystem declares all of the related interfaces that must be implemented
// by a file system implementation as well as a number of static helper methods
// that can be used to more easily access file system functionality.
//
// Base classes are provided to define common behaviors that can be leveraged
// to assist slightly in a file system implementation.

struct __declspec(novtable) FileSystem
{
	// Forward Interface Declarations
	//
	struct __declspec(novtable) Alias;
	struct __declspec(novtable) BlockDevice;
	struct __declspec(novtable) CharacterDevice;
	struct __declspec(novtable) Directory;
	struct __declspec(novtable) File;
	struct __declspec(novtable) Handle;
	struct __declspec(novtable) Mount;
	struct __declspec(novtable) Node;
	struct __declspec(novtable) Pipe;
	struct __declspec(novtable) Socket;
	struct __declspec(novtable) SymbolicLink;

	// Forward Class Declarations
	//
	class ExecuteHandle;
	class Path;
	class PathHandle;

	// MountFunction
	//
	// Function signature for a file system's Mount() implementation, which must be a public static method
	using MountFunction = std::function<std::shared_ptr<Mount>(const char_t* source, uint32_t flags, const void* data, size_t datalength)>;

	// FileSystem::NodeType
	//
	// Strogly typed enumeration for the S_IFxxx inode type constants
	enum class NodeType
	{
		BlockDevice			= LINUX_S_IFBLK,
		CharacterDevice		= LINUX_S_IFCHR,
		Directory			= LINUX_S_IFDIR,
		File				= LINUX_S_IFREG,
		Pipe				= LINUX_S_IFIFO,
		Socket				= LINUX_S_IFSOCK,
		SymbolicLink		= LINUX_S_IFLNK,
	};

	// FileSystem::Permission (bitmask)
	//
	// Strongly typed enumeration for generic permission flags
	enum class Permission
	{
		Execute				= 0x01,
		Write				= 0x02,
		Read				= 0x04,
	};

	// FileSystem::Mount
	//
	// Interface that must be implemented by a file system mount.  A mount is a view
	// of a file system within a namespace
	struct __declspec(novtable) Mount
	{
		// Duplicate
		//
		// Duplicates this mount instance
		virtual std::shared_ptr<Mount> Duplicate(void) = 0;

		// Remount
		//
		// Remounts this mount point with different flags and arguments
		virtual void Remount(uint32_t flags, const void* data, size_t datalen) = 0;
	
		// Stat
		//
		// Provides statistical information about the file system
		virtual void Stat(uapi::statfs* stats) = 0;

		// Flags
		//
		// Gets the flags set on this mount, includes file system flags
		__declspec(property(get=getFlags)) uint32_t Flags;
		virtual uint32_t getFlags(void) = 0;

		// Root
		//
		// Gets a reference to the root node of the mount point
		__declspec(property(get=getRoot)) std::shared_ptr<Node> Root;
		virtual std::shared_ptr<Node> getRoot(void) = 0;

		// Source
		//
		// Gets the device/name used as the source of the file system
		__declspec(property(get=getSource)) const char_t* Source;
		virtual const char_t* getSource(void) = 0;
	};

	// FileSystem::Alias
	//
	// Interface that must be implemented by a file system alias.  An alias defines a name
	// that is associated with a file system node.  An alias may also serve as a mount point
	// in the file system hierarchy; mounting a node to an existing alias will obscure the
	// original node reference within that namespace. All aliases must have a parent, in the
	// case of a root node, the parent would be self-referential
	struct __declspec(novtable) Alias
	{
		// Follow
		//
		// Follows this alias to the file system node that it refers to
		virtual std::shared_ptr<Node> Follow(const std::shared_ptr<Namespace>& ns) = 0;

		// PopMount
		//
		// Removes a mountpoint node from this alias
		virtual void PopMount(const std::shared_ptr<Namespace>& ns) = 0;

		// PushMount
		//
		// Adds a mountpoint node to this alias, obscuring any existing node in the same namespace
		virtual void PushMount(const std::shared_ptr<Namespace>& ns, const std::shared_ptr<struct Mount>& mount) = 0;

		// Mount
		//
		// Retrieves the mountpoint instance for the specified namespace, or nullptr if not mounted
		__declspec(property(get=getMount)) std::shared_ptr<FileSystem::Mount> Mount[];
		virtual std::shared_ptr<FileSystem::Mount> getMount(const std::shared_ptr<Namespace>& ns) = 0;

		// Mounted
		//
		// Determines if this alias serves as a mountpoint for the specified namespace
		__declspec(property(get=getMounted)) bool Mounted[];
		virtual bool getMounted(const std::shared_ptr<Namespace>& ns) = 0;

		// Name
		//
		// Gets the name associated with the alias
		__declspec(property(get=getName)) const char_t* Name;
		virtual const char_t* getName(void) = 0;

		// Parent
		//
		// Gets the parent alias of this alias instance, or nullptr if none exists
		__declspec(property(get=getParent)) std::shared_ptr<Alias> Parent;
		virtual std::shared_ptr<Alias> getParent(void) = 0;		
	};

	// FileSystem::Path
	//
	// Path represents the means to fully reference a file system object. While an Alias
	// provides the name->object mapping, the active Mount instance is also required in
	// order to check file system permissions, which can vary per mount point
	class Path
	{
	public:

		// Create
		//
		// Creates a new Path instance from component Mount and Alias instances
		static std::unique_ptr<Path> Create(const std::shared_ptr<FileSystem::Mount>& mount, const std::shared_ptr<FileSystem::Alias>& alias);

		// Duplicate
		//
		// Duplicates this Path instance
		std::unique_ptr<Path> Duplicate(void) const;

		// Alias
		//
		// Gets a reference to the contained Alias instance
		__declspec(property(get=getAlias)) std::shared_ptr<FileSystem::Alias> Alias;
		std::shared_ptr<FileSystem::Alias> getAlias(void) const;

		// Mount
		//
		// Gets a reference to the contained Mount instance
		__declspec(property(get=getMount)) std::shared_ptr<FileSystem::Mount> Mount;
		std::shared_ptr<FileSystem::Mount> getMount(void) const;

	private:

		Path(const Path&)=delete;
		Path& operator=(const Path&)=delete;

		// Instance Constructor
		//
		Path(const std::shared_ptr<struct Mount>& mount, const std::shared_ptr<struct Alias>& alias);
		friend std::unique_ptr<Path> std::make_unique<Path, const std::shared_ptr<struct Mount>&, const std::shared_ptr<struct Alias>&>
			(const std::shared_ptr<struct Mount>& mount, const std::shared_ptr<struct Alias>& alias);

		//---------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<struct Alias>		m_alias;	// Alias reference
		const std::shared_ptr<struct Mount>		m_mount;	// Mount reference
	};

	// FileSystem::Handle
	//
	// Handle is the interface that must be implemented for a file system handle object.
	// A handle is a view of a file system node
	struct __declspec(novtable) Handle
	{
		// Duplicate
		//
		// Creates a duplicate Handle instance
		virtual std::shared_ptr<Handle> Duplicate(int flags) = 0;

		// Read
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t Read(void* buffer, uapi::size_t count) = 0;

		// ReadAt
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count) = 0;

		// Seek
		//
		// Changes the file position
		virtual uapi::loff_t Seek(uapi::loff_t offset, int whence) = 0;

		// Sync
		//
		// Synchronizes all metadata and data associated with the file to storage
		virtual void Sync(void) = 0;

		// SyncData
		//
		// Synchronizes all data associated with the file to storage, not metadata
		virtual void SyncData(void) = 0;

		// Write
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t Write(const void* buffer, uapi::size_t count) = 0;

		// WriteAt
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count) = 0;

		//// Alias
		////
		//// Gets a reference to the Alias used to open this handle
		//__declspec(property(get=getAlias)) std::shared_ptr<FileSystem::Alias> Alias;
		//virtual std::shared_ptr<FileSystem::Alias> getAlias(void) = 0;

		//// CloseOnExec
		////
		//// Gets/sets the flag to close this handle during an execute operation
		//__declspec(property(get=getCloseOnExec, put=putCloseOnExec)) bool CloseOnExec;
		//virtual bool getCloseOnExec(void) = 0;
		//virtual void putCloseOnExec(bool value) = 0;

		//// Flags
		////
		//// Gets a copy of the current handle flags
		//__declspec(property(get=getFlags)) int Flags;
		//virtual int getFlags(void) = 0;

		//// Node
		////
		//// Gets the node instance to which this handle refers
		//__declspec(property(get=getNode)) std::shared_ptr<FileSystem::Node> Node;
		//virtual std::shared_ptr<FileSystem::Node> getNode(void) = 0;
	};

	// FileSystem::Node
	//
	// Interface that must be implemented for a file system node object.  A node represents
	// any object (file, directory, socket, etc) that is a member of a file system
	struct __declspec(novtable) Node
	{
		// DemandPermission
		//
		// Demands combination of read/write/execute permissions for the node
		virtual void DemandPermission(Permission mask) = 0;

		// Lookup
		//
		// Resolves a file system path using this node as the starting point
		virtual std::unique_ptr<FileSystem::Path> Lookup(const std::shared_ptr<Namespace>& ns, const std::unique_ptr<Path>& root, 
			const std::unique_ptr<Path>& current, const char_t* path, int flags, int* reparses) = 0;
		
		// Open
		//
		// Creates a Handle instance against this node
		virtual std::shared_ptr<Handle> Open(const std::unique_ptr<Path>& thispath, int flags) = 0;

		// Stat
		//
		// Provides statistical information about the node
		virtual void Stat(uapi::stat* stats) = 0;

		// Type
		//
		// Gets the type of node being represented in the derived object instance
		__declspec(property(get=getType)) NodeType Type;
		virtual NodeType getType(void) = 0;
	};

	// FileSystem::BlockDevice
	//
	// Specialization of Node for file system block device objects
	struct __declspec(novtable) BlockDevice : public Node
	{
	};

	// FileSystem::CharacterDevice
	//
	// Specialization of Node for file system character device objects
	struct __declspec(novtable) CharacterDevice : public Node
	{
	};

	// FileSystem::Directory
	//
	// Specialization of Node for file system directory objects
	struct __declspec(novtable) Directory : public Node
	{
		// CreateCharacterDevice
		//
		// Creates a new character device node as a child of this node
		virtual void CreateCharacterDevice(const std::unique_ptr<Path>& thispath, const char_t* name, uapi::mode_t mode, uapi::dev_t device) = 0;

		// CreateDirectory
		//
		// Creates a new directory node as a child of this node
		virtual void CreateDirectory(const std::unique_ptr<Path>& thispath, const char_t* name, uapi::mode_t mode) = 0;

		// CreateFile
		//
		// Creates a new regular file node as a child of this node
		virtual std::shared_ptr<Handle> CreateFile(const std::unique_ptr<Path>& thispath, const char_t* name, int flags, uapi::mode_t mode) = 0;

		// CreateSymbolicLink
		//
		// Creates a new symbolic link as a child of this node
		virtual void CreateSymbolicLink(const std::unique_ptr<Path>& thispath, const char_t* name, const char_t* target) = 0;
	};

	// FileSystem::File
	//
	// Specialization of Node for file system regular file objects
	struct __declspec(novtable) File : public Node
	{
		// OpenExec
		//
		// Creates a FileSystem::Handle instance for this node, specifically for use
		// by the virtual machine as part of process creation
		virtual std::shared_ptr<Handle> OpenExec(const std::shared_ptr<Alias>& alias) = 0;
	};

	// FileSystem::Pipe
	//
	// Specialization of Node for file system pipe objects
	struct __declspec(novtable) Pipe : public Node
	{
	};

	// FileSystem::Socket
	//
	// Specialization of Node for file system socket objects
	struct __declspec(novtable) Socket : public Node
	{
	};

	// FileSystem::SymbolicLink
	//
	// Specialization of Node for file system symbolic link objects
	struct __declspec(novtable) SymbolicLink : public Node
	{
		// ReadTarget
		//
		// Reads the target of the symbolic link
		virtual uapi::size_t ReadTarget(char_t* buffer, size_t count) = 0; 
	};

	// FileSystem::ExecuteHandle
	//
	// Specialized handle wrapper used for handles generated for the purpose
	// of executing a binary from the file system.  Construct around a custom
	// file system handle with the Create() method.
	class ExecuteHandle : public Handle
	{
	public:

		// Destructor
		//
		~ExecuteHandle()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Create (static)
		//
		// Constructs an ExecuteHandle wrapper around an existing handle instance
		static std::shared_ptr<Handle> Create(const std::shared_ptr<Handle>& handle);

		//---------------------------------------------------------------------
		// FileSystem::Handle Implementation

		// Duplicate
		//
		// Creates a duplicate Handle instance
		virtual std::shared_ptr<Handle> Duplicate(int flags);

		// Read
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t Read(void* buffer, uapi::size_t count);

		// ReadAt
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count);

		// Seek
		//
		// Changes the file position
		virtual uapi::loff_t Seek(uapi::loff_t offset, int whence);

		// Sync
		//
		// Synchronizes all metadata and data associated with the file to storage
		virtual void Sync(void);

		// SyncData
		//
		// Synchronizes all data associated with the file to storage, not metadata
		virtual void SyncData(void);

		// Write
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t Write(const void* buffer, uapi::size_t count);

		// WriteAt
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count);

	private:

		ExecuteHandle(const ExecuteHandle&)=delete;
		ExecuteHandle& operator=(const ExecuteHandle&)=delete;

		// Instance Constructor
		//
		ExecuteHandle(const std::shared_ptr<Handle>& handle);
		friend class std::_Ref_count_obj<ExecuteHandle>;

		//---------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<Handle>	m_handle;	// Contained handle instance
	};

	// FileSystem::PathHandle
	//
	// Specialized Handle wrapper used for handles opened for O_PATH purposes.
	// Construct around a custom file system handle with the Create() method.
	class PathHandle : public Handle
	{
	public:

		// Destructor
		//
		~PathHandle()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Create (static)
		//
		// Constructs an ExecuteHandle wrapper around an existing handle instance
		static std::shared_ptr<Handle> Create(const std::shared_ptr<Handle>& handle);

		//---------------------------------------------------------------------
		// FileSystem::Handle Implementation

		// Duplicate
		//
		// Creates a duplicate Handle instance
		virtual std::shared_ptr<Handle> Duplicate(int flags);

		// Read
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t Read(void* buffer, uapi::size_t count);

		// ReadAt
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count);

		// Seek
		//
		// Changes the file position
		virtual uapi::loff_t Seek(uapi::loff_t offset, int whence);

		// Sync
		//
		// Synchronizes all metadata and data associated with the file to storage
		virtual void Sync(void);

		// SyncData
		//
		// Synchronizes all data associated with the file to storage, not metadata
		virtual void SyncData(void);

		// Write
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t Write(const void* buffer, uapi::size_t count);

		// WriteAt
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count);

	private:

		PathHandle(const PathHandle&)=delete;
		PathHandle& operator=(const PathHandle&)=delete;

		// Instance Constructor
		//
		PathHandle(const std::shared_ptr<Handle>& handle);
		friend class std::_Ref_count_obj<PathHandle>;

		//---------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<Handle>	m_handle;	// Contained handle instance
	};

	//
	// FILE SYSTEM API
	//

	static void CheckPermissions(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, 
		int flags, uapi::mode_t mode);

	static void CreateCharacterDevice(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base,
		const char_t* path, uapi::mode_t mode, uapi::dev_t device);

	static void CreateDirectory(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, uapi::mode_t mode);

	static std::shared_ptr<Handle> CreateFile(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, 
		int flags, uapi::mode_t mode);

	static void CreateSymbolicLink(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, const char_t* target);

	// GenerateFileSystemId
	//
	// Generates a unique file system identifier (fsid)
	static uapi::fsid_t GenerateFileSystemId(void);

	static void GetAbsolutePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& alias, char_t* path, size_t pathlen);

	static std::shared_ptr<Handle> OpenExecutable(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path);

	static std::shared_ptr<Handle> OpenFile(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base,
		const char_t* path, int flags, uapi::mode_t mode);

	static size_t ReadSymbolicLink(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, 
		char_t* buffer, size_t length);

	static std::shared_ptr<Alias> ResolvePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, int flags);
};

// FileSystem::Permission bitwise operators
//
inline FileSystem::Permission operator~(FileSystem::Permission lhs) {
	return static_cast<FileSystem::Permission>(~static_cast<uint32_t>(lhs));
}

inline FileSystem::Permission operator&(FileSystem::Permission lhs, FileSystem::Permission rhs) {
	return static_cast<FileSystem::Permission>(static_cast<uint32_t>(lhs) & (static_cast<uint32_t>(rhs)));
}

inline FileSystem::Permission operator|(FileSystem::Permission lhs, FileSystem::Permission rhs) {
	return static_cast<FileSystem::Permission>(static_cast<uint32_t>(lhs) | (static_cast<uint32_t>(rhs)));
}

inline FileSystem::Permission operator^(FileSystem::Permission lhs, FileSystem::Permission rhs) {
	return static_cast<FileSystem::Permission>(static_cast<uint32_t>(lhs) ^ (static_cast<uint32_t>(rhs)));
}

// FileSystem::Permission compound assignment operators
//
inline FileSystem::Permission& operator&=(FileSystem::Permission& lhs, FileSystem::Permission rhs) 
{
	lhs = lhs & rhs;
	return lhs;
}

inline FileSystem::Permission& operator|=(FileSystem::Permission& lhs, FileSystem::Permission rhs) 
{
	lhs = lhs | rhs;
	return lhs;
}

inline FileSystem::Permission& operator^=(FileSystem::Permission& lhs, FileSystem::Permission rhs) 
{
	lhs = lhs ^ rhs;
	return lhs;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM_H_