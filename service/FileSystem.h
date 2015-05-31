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
	// Forward Declarations
	//
	struct __declspec(novtable) Alias;
	struct __declspec(novtable) BlockDevice;
	struct __declspec(novtable) CharacterDevice;
	struct __declspec(novtable) Directory;
	struct __declspec(novtable) File;
	struct __declspec(novtable) Handle;
	struct __declspec(novtable) Mount;
	struct __declspec(novtable) MountOptions;
	struct __declspec(novtable) Node;
	struct __declspec(novtable) Pipe;
	struct __declspec(novtable) Socket;
	struct __declspec(novtable) SymbolicLink;

	// MountFunction
	//
	// Function signature for a file system's Mount() implementation, which must be a public static method
	using MountFunction = std::function<std::shared_ptr<Mount>(const char_t* device, uint32_t flags, const void* data, size_t datalength)>;

	// NodeType
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


	//// NODE_INDEX_ROOT
	////
	//// Constant indicating the node index for a file system root node
	//static const uapi::ino_t NODE_INDEX_ROOT = 2;

	//// NODE_INDEX_LOSTANDFOUND
	////
	//// Constant indicating the node index for a lost+found directory node
	//static const uapi::ino_t NODE_INDEX_LOSTANDFOUND = 3;

	//// NODE_INDEX_FIRSTDYNAMIC
	////
	//// Constant indicating the first dynamic node index that should be used
	//static const uapi::ino_t NODE_INDEX_FIRSTDYNAMIC = 4;

	//// MAXIMUM_PATH_SYMLINKS
	////
	//// The maximum number of symbolic links that can exist in a path
	//static const int MAXIMUM_PATH_SYMLINKS = 40;

	// FileSystem::MountOptions
	//
	struct __declspec(novtable) MountOptions
	{
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

		// FileSystem
		//
		// Gets a reference to the underlying file system instance
		__declspec(property(get=getFileSystem)) std::shared_ptr<::FileSystem> FileSystem;
		virtual std::shared_ptr<::FileSystem> getFileSystem(void) = 0;

		// Options
		//
		// Gets a reference to the current set of options associated with this mount
		__declspec(property(get=getOptions)) std::shared_ptr<MountOptions> Options;
		virtual std::shared_ptr<MountOptions> getOptions(void) = 0;
	};

	// FileSystem::Alias
	//
	// Interface that must be implemented by a file system alias.  An alias defines a name
	// that is associated with a file system node.  An alias may also serve as a mount point
	// in the file system hierarchy; mounting a node to an existing alias will obscure the
	// original node reference within that namespace
	struct __declspec(novtable) Alias
	{
		// Follow
		//
		// Follows this alias to the file system node that it refers to
		virtual std::shared_ptr<Node> Follow(const std::shared_ptr<Namespace>& ns) = 0;

		// Mount
		//
		// Adds a mountpoint node to this alias, obscuring any existing node in the same namespace
		virtual void Mount(const std::shared_ptr<Namespace>& ns, const std::shared_ptr<Node>& node) = 0;

		// Unmount
		//
		// Removes a mountpoint node from this alias
		virtual void Unmount(const std::shared_ptr<Namespace>& ns, const std::shared_ptr<Node>& node) = 0;

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

	// FileSystem::Handle
	//
	// Handle is the interface that must be implemented for a file system handle object.
	// A handle is a view of a file system node instance, for intention of reading/writing
	// information to/from that node.
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

		// Alias
		//
		// Gets a reference to the Alias used to open this handle
		__declspec(property(get=getAlias)) std::shared_ptr<FileSystem::Alias> Alias;
		virtual std::shared_ptr<FileSystem::Alias> getAlias(void) = 0;

		// CloseOnExec
		//
		// Gets/sets the flag to close this handle during an execute operation
		__declspec(property(get=getCloseOnExec, put=putCloseOnExec)) bool CloseOnExec;
		virtual bool getCloseOnExec(void) = 0;
		virtual void putCloseOnExec(bool value) = 0;

		// Flags
		//
		// Gets a copy of the current handle flags
		__declspec(property(get=getFlags)) int Flags;
		virtual int getFlags(void) = 0;

		// Node
		//
		// Gets the node instance to which this handle refers
		__declspec(property(get=getNode)) std::shared_ptr<FileSystem::Node> Node;
		virtual std::shared_ptr<FileSystem::Node> getNode(void) = 0;
	};

	// FileSystem::Node
	//
	// Interface that must be implemented for a file system node object.  A node represents
	// any object (file, directory, socket, etc) that is part of a file system
	struct __declspec(novtable) Node
	{
		// DemandPermission
		//
		// Demands read/write/execute permissions for the node (MAY_READ, MAY_WRITE, MAY_EXECUTE)
		virtual void DemandPermission(uapi::mode_t mode) = 0;

		// Open
		//
		// Creates a Handle instance against this node
		virtual std::shared_ptr<Handle> Open(const std::shared_ptr<Alias>& alias, int flags) = 0;

		// Lookup
		//
		// Resolves a relative path to an alias from this node
		virtual std::shared_ptr<Alias> Lookup(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& current, 
			const char_t* path, int flags, int* symlinks) = 0;
		
		// Stat
		//
		// Provides statistical information about the node
		virtual void Stat(uapi::stat* stats) = 0;

		// FileSystem
		//
		// Gets a reference to this node's parent file system instance
		__declspec(property(get=getFileSystem)) std::shared_ptr<::FileSystem> FileSystem;
		virtual std::shared_ptr<::FileSystem> getFileSystem(void) = 0;

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
		virtual void CreateCharacterDevice(const std::shared_ptr<Alias>& parent, const char_t* name, uapi::mode_t mode, uapi::dev_t device) = 0;

		// CreateDirectory
		//
		// Creates a new directory node as a child of this node
		virtual void CreateDirectory(const std::shared_ptr<Alias>& parent, const char_t* name, uapi::mode_t mode) = 0;

		// CreateFile
		//
		// Creates a new regular file node as a child of this node
		virtual std::shared_ptr<Handle> CreateFile(const std::shared_ptr<Alias>& parent, const char_t* name, int flags, uapi::mode_t mode) = 0;

		// CreateSymbolicLink
		//
		// Creates a new symbolic link as a child of this node
		virtual void CreateSymbolicLink(const std::shared_ptr<Alias>& parent, const char_t* name, const char_t* target) = 0;
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
	struct __declspec(novtable) SymbolicLink : public virtual Node
	{
		// ReadTarget
		//
		// Reads the target of the symbolic link
		virtual uapi::size_t ReadTarget(char_t* buffer, size_t count) = 0; 
	};

	// Stat
	//
	// Provides statistical information about the file system
	virtual void Stat(uapi::statfs* stats) = 0;

	// Root
	//
	// Gets a reference to the root file system node
	__declspec(property(get=getRoot)) std::shared_ptr<Node> Root;
	virtual std::shared_ptr<Node> getRoot(void) = 0;

	// Source
	//
	// Gets the device/name used as the source of the file system
	__declspec(property(get=getSource)) const char_t* Source;
	virtual const char_t* getSource(void) = 0;

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

	static void GetAbsolutePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& alias, char_t* path, size_t pathlen);

	static std::shared_ptr<Handle> OpenExecutable(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path);

	static std::shared_ptr<Handle> OpenFile(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base,
		const char_t* path, int flags, uapi::mode_t mode);

	static size_t ReadSymbolicLink(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, 
		char_t* buffer, size_t length);

	static std::shared_ptr<Alias> ResolvePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, int flags);

	//
	// BASE CLASSES
	//

	// FileSystem::AliasBase
	//
	// todo: what is implemented
	class AliasBase : public Alias
	{
	public:

		// Destructor
		//
		virtual ~AliasBase()=default;

	protected:

		// Instance Constructor
		//
		AliasBase()=default;

	private:

		AliasBase(const AliasBase&)=delete;
		AliasBase& operator=(const AliasBase&)=delete;
	};

	// FileSystem::BlockDeviceBase
	//
	// todo: what is implemented by this base class
	class BlockDeviceBase : public BlockDevice
	{
	public:

		// Destructor
		//
		virtual ~BlockDeviceBase()=default;

	protected:

		// Instance Constructor
		//
		BlockDeviceBase()=default;

	private:

		BlockDeviceBase(const BlockDeviceBase&)=delete;
		BlockDeviceBase& operator=(const BlockDeviceBase&)=delete;

		// Node::getType
		//
		// Gets the type of node defined by the derived class
		virtual NodeType getType(void);
	};

	// FileSystem::CharacterDeviceBase
	//
	// todo: what is implemented by this base class
	class CharacterDeviceBase : public CharacterDevice
	{
	public:

		// Destructor
		//
		virtual ~CharacterDeviceBase()=default;

	protected:

		// Instance Constructor
		//
		CharacterDeviceBase()=default;

	private:

		CharacterDeviceBase(const CharacterDeviceBase&)=delete;
		CharacterDeviceBase& operator=(const CharacterDeviceBase&)=delete;

		// Node::getType
		//
		// Gets the type of node defined by the derived class
		virtual NodeType getType(void);
	};

	// FileSystem::DirectoryBase
	//
	// todo: what is implemented by this base class
	class DirectoryBase : public Directory
	{
	public:

		// Destructor
		//
		virtual ~DirectoryBase()=default;

	protected:

		// Instance Constructor
		//
		DirectoryBase()=default;

	private:

		DirectoryBase(const DirectoryBase&)=delete;
		DirectoryBase& operator=(const DirectoryBase&)=delete;

		// Node::getType
		//
		// Gets the type of node defined by the derived class
		virtual NodeType getType(void);
	};

	// FileSystem::FileBase
	//
	// todo: what is implemented by this base class
	class FileBase : public File
	{
	public:

		// Destructor
		//
		virtual ~FileBase()=default;

	protected:

		// Instance Constructor
		//
		FileBase()=default;

	private:

		FileBase(const FileBase&)=delete;
		FileBase& operator=(const FileBase&)=delete;

		// Node::getType
		//
		// Gets the type of node defined by the derived class
		virtual NodeType getType(void);
	};

	// FileSystem::HandleBase
	//
	// todo: what is implemented
	class HandleBase : public Handle
	{
	public:

		// Destructor
		//
		virtual ~HandleBase()=default;

	protected:

		// Instance Constructor
		//
		HandleBase()=default;

	private:

		HandleBase(const HandleBase&)=delete;
		HandleBase& operator=(const HandleBase&)=delete;
	};

	// FileSystem::MountBase
	//
	// todo: what is implemented
	class MountBase : public Mount
	{
	public:

		// Destructor
		//
		virtual ~MountBase()=default;

	protected:

		// Instance Constructor
		//
		MountBase()=default;

	private:

		MountBase(const MountBase&)=delete;
		MountBase& operator=(const MountBase&)=delete;
	};

	// FileSystem::MountOptionsBase
	//
	// todo: what is implemented
	class MountOptionsBase : public MountOptions
	{
	public:

		// Destructor
		//
		virtual ~MountOptionsBase()=default;

	protected:

		// Instance Constructor
		//
		MountOptionsBase()=default;

	private:

		MountOptionsBase(const MountOptionsBase&)=delete;
		MountOptionsBase& operator=(const MountOptionsBase&)=delete;
	};

	// FileSystem::PipeBase
	//
	// todo: what is implemented by this base class
	class PipeBase : public Pipe
	{
	public:

		// Destructor
		//
		virtual ~PipeBase()=default;

	protected:

		// Instance Constructor
		//
		PipeBase()=default;

	private:

		PipeBase(const PipeBase&)=delete;
		PipeBase& operator=(const PipeBase&)=delete;

		// Node::getType
		//
		// Gets the type of node defined by the derived class
		virtual NodeType getType(void);
	};

	// FileSystem::SocketBase
	//
	// todo: what is implemented by this base class
	class SocketBase : public Socket
	{
	public:

		// Destructor
		//
		virtual ~SocketBase()=default;

	protected:

		// Instance Constructor
		//
		SocketBase()=default;

	private:

		SocketBase(const SocketBase&)=delete;
		SocketBase& operator=(const SocketBase&)=delete;

		// Node::getType
		//
		// Gets the type of node defined by the derived class
		virtual NodeType getType(void);
	};

	// FileSystem::SymbolicLinkBase
	//
	// todo: what is implemented by this base class
	class SymbolicLinkBase : public SymbolicLink
	{
	public:

		// Destructor
		//
		virtual ~SymbolicLinkBase()=default;

	protected:

		// Instance Constructor
		//
		SymbolicLinkBase()=default;

	private:

		SymbolicLinkBase(const SymbolicLinkBase&)=delete;
		SymbolicLinkBase& operator=(const SymbolicLinkBase&)=delete;

		// Node::getType
		//
		// Gets the type of node defined by the derived class
		virtual NodeType getType(void);
	};
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM_H_