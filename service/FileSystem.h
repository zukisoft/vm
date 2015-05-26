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

#include <vector>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <linux/statfs.h>
#include <linux/types.h>
#include "LinuxException.h"
#include "MountOptions.h"
#include "PathSplitter.h"

#pragma warning(push, 4)

// todo: document
//
// todo: consider making everything in here lowercase filesystem_ptr vs FileSystemPtr, for example?

class FileSystem;
using FileSystemPtr = std::shared_ptr<FileSystem>;

//-----------------------------------------------------------------------------
// FileSystem
//
// todo: words

class FileSystem
{
public:

	// Instance Constructor
	//
	FileSystem()=default;

	// AliasPtr
	//
	// Alias for an std::shared_ptr<Alias>
	struct Alias;
	using AliasPtr = std::shared_ptr<Alias>;

	// DirectoryPtr
	//
	// Alias for an std::shared_ptr<Directory>
	struct Directory;
	using DirectoryPtr = std::shared_ptr<Directory>;

	// HandlePtr
	//
	// Alias for an std::shared_ptr<Handle>
	struct Handle;
	using HandlePtr = std::shared_ptr<Handle>;

	// NodePtr
	//
	// Alias for an std::shared_ptr<Node>
	struct Node;
	using NodePtr = std::shared_ptr<Node>;

	// SymbolicLinkPtr
	//
	// Alias for an std::shared_ptr<SymbolicLink>
	struct SymbolicLink;
	using SymbolicLinkPtr = std::shared_ptr<SymbolicLink>;

	// need typedef for Mount(const uapi::char_t* device, uint32_t flags, const void* data)
	// need table type for mountable file systems -> Mount() function pointers
	//using mount_func = std::function<FileSystemPtr(const uapi::char_t* device, uint32_t flags, const void* data, size_t length)>;
	using mount_func = std::function<FileSystemPtr(const char_t* device, std::unique_ptr<MountOptions>&& options)>;
	using MountFunction = mount_func;

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

	// NODE_INDEX_ROOT
	//
	// Constant indicating the node index for a file system root node
	static const uapi::ino_t NODE_INDEX_ROOT = 2;

	// NODE_INDEX_LOSTANDFOUND
	//
	// Constant indicating the node index for a lost+found directory node
	static const uapi::ino_t NODE_INDEX_LOSTANDFOUND = 3;

	// NODE_INDEX_FIRSTDYNAMIC
	//
	// Constant indicating the first dynamic node index that should be used
	static const uapi::ino_t NODE_INDEX_FIRSTDYNAMIC = 4;

	// MAXIMUM_PATH_SYMLINKS
	//
	// The maximum number of symbolic links that can exist in a path
	static const int MAXIMUM_PATH_SYMLINKS = 40;

	//-------------------------------------------------------------------------
	// FileSystem::Alias
	//
	// Alias implements a file system object name that points to an underlying node
	struct __declspec(novtable) Alias
	{
		// Mount
		//
		// Mounts/binds a foreign node to this alias, obscuring the previous node
		virtual void Mount(const NodePtr& node) = 0;

		// Unmount
		//
		// Unmounts/unbinds a node from this alias, revealing the previously bound node
		virtual void Unmount(void) = 0;

		// Name
		//
		// Gets the name associated with this alias
		__declspec(property(get=getName)) const uapi::char_t* Name;
		virtual const uapi::char_t* getName(void) = 0;

		// Node
		//
		// Gets the node instance to which this alias references
		__declspec(property(get=getNode)) NodePtr Node;
		virtual NodePtr getNode(void) = 0;

		// Parent
		//
		// Gets the parent Alias instance for this alias, may be nullptr
		// if the parent has been released/removed or otherwise doesn't exist
		__declspec(property(get=getParent)) AliasPtr Parent;
		virtual AliasPtr getParent(void) = 0;
	};

	// Mount
	//
	struct __declspec(novtable) Mount
	{
		// Duplicate
		//
		// Duplicates this mount instance
		virtual std::shared_ptr<Mount> Duplicate(void) const = 0;

		// Remount
		//
		// Remounts this mount point with different flags and arguments
		virtual void Remount(uint32_t flags, const void* data, size_t datalen) = 0;

		// Options
		//
		// Gets a pointer to the contained MountOptions instance
		__declspec(property(get=getOptions)) const MountOptions* Options;
		virtual const MountOptions* getOptions(void) const = 0;

		// Root
		//
		// Gets a pointer to the root node of this mount point
		__declspec(property(get=getRoot)) std::shared_ptr<Node> Root;
		virtual std::shared_ptr<Node> getRoot(void) const = 0;

		// Source
		//
		// Retrieves the source device name for the mount point
		__declspec(property(get=getSource)) const char_t* Source;
		virtual const char_t* getSource(void) const = 0;
	};

	// Node
	//
	// todo: need permission arguments (mode_t)
	struct __declspec(novtable) Node
	{
		// DemandPermission
		//
		// Demands read/write/execute permissions for the file system object (MAY_READ, MAY_WRITE, MAY_EXECUTE)
		virtual void DemandPermission(uapi::mode_t mode) = 0;

		// Open
		//
		// Creates a FileSystem::Handle instance for this node
		virtual HandlePtr Open(const AliasPtr& alias, int flags) = 0;

		// Resolve
		//
		// Resolves a relative path from this node to an Alias instance
		// TODO: too many arguments, create a state object.  Resolve() is recursive, don't blow up the stack
		// would like to swap root and current, but compiler won't catch that if I do and weird stuff will happen,
		// that will take care of itself if I make a state object that has these instead
		virtual AliasPtr Resolve(const AliasPtr& root, const AliasPtr& current, const uapi::char_t* path, int flags, int* symlinks) = 0;

		// FileSystem
		//
		// Gets a reference to this node's parent FileSystem instance
		__declspec(property(get=getFileSystem)) std::shared_ptr<class FileSystem> FileSystem;
		virtual std::shared_ptr<class FileSystem> getFileSystem(void) = 0;

		// Index
		//
		// Gets the node index
		//__declspec(property(get=getIndex)) uint64_t Index;
		//virtual uint64_t getIndex(void) = 0;

		// Status
		//
		// Gets the node status information
		__declspec(property(get=getStatus)) uapi::stat Status;
		virtual uapi::stat getStatus(void) = 0;

		// Type
		//
		// Gets the node type
		__declspec(property(get=getType)) NodeType Type;
		virtual NodeType getType(void) = 0;
	};

	// Directory
	//
	// Specialization of Node for Directory objects
	struct __declspec(novtable) Directory : public Node
	{
		// CreateCharacterDevice
		//
		// Creates a new character device node as a child of this node
		virtual void CreateCharacterDevice(const AliasPtr& parent, const uapi::char_t* name, uapi::mode_t mode, uapi::dev_t device) = 0;

		// CreateDirectory
		//
		// Creates a new directory node as a child of this node
		virtual void CreateDirectory(const AliasPtr& parent, const uapi::char_t* name, uapi::mode_t mode) = 0;

		// CreateFile
		//
		// Creates a new regular file node as a child of this node
		virtual HandlePtr CreateFile(const AliasPtr& parent, const uapi::char_t* name, int flags, uapi::mode_t mode) = 0;

		// CreateSymbolicLink
		//
		// Creates a new symbolic link as a child of this node
		virtual void CreateSymbolicLink(const AliasPtr& parent, const uapi::char_t* name, const uapi::char_t* target) = 0;

		// RemoveDirectory
		//
		// Removes a directory child from the node
		//virtual void RemoveDirectory(const uapi::char_t* name) = 0;

		// RemoveNode
		//
		// Removes a non-directory child from the node
		//virtual void RemoveNode(const uapi::char_t* name) = 0;
	};

	// CharacterDevice
	//
	// Specialization of Node for Character Device objects
	struct __declspec(novtable) CharacterDevice : public Node
	{
	};

	// File
	//
	// Specialization of Node for File objects
	struct __declspec(novtable) File : public Node
	{
		// OpenExec
		//
		// Creates a FileSystem::Handle instance for this node, specifically for use
		// by the virtual machine as part of process creation
		virtual HandlePtr OpenExec(const AliasPtr& alias) = 0;
	};

	// SymbolicLink
	//
	// Specialization of Node for Symbolic Link objects
	struct __declspec(novtable) SymbolicLink : public virtual Node
	{
		// ReadTarget
		//
		// Reads the target of the symbolic link
		virtual uapi::size_t ReadTarget(uapi::char_t* buffer, size_t count) = 0; 
	};

	// Handle
	//
	// todo: document when done
	struct __declspec(novtable) Handle
	{
		// Duplicate
		//
		// Creates a duplicate Handle instance
		virtual HandlePtr Duplicate(int flags) = 0;

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
		// Gets a reference to the Alias instance used to open this handle
		__declspec(property(get=getAlias)) AliasPtr Alias;
		virtual AliasPtr getAlias(void) = 0;

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
		// Gets the node instance to which this alias references
		__declspec(property(get=getNode)) NodePtr Node;
		virtual NodePtr getNode(void) = 0;
	};

	//
	// FileSystem Members
	//

	static void CheckPermissions(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path, 
		int flags, uapi::mode_t mode);

	static void CreateCharacterDevice(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base,
		const uapi::char_t* path, uapi::mode_t mode, uapi::dev_t device);

	static void CreateDirectory(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path, uapi::mode_t mode);

	static std::shared_ptr<Handle> CreateFile(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path, 
		int flags, uapi::mode_t mode);

	static void CreateSymbolicLink(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path, const uapi::char_t* target);

	static void GetAbsolutePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& alias, uapi::char_t* path, size_t pathlen);

	static std::shared_ptr<Handle> OpenExecutable(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path);

	static std::shared_ptr<Handle> OpenFile(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base,
		const uapi::char_t* path, int flags, uapi::mode_t mode);

	static size_t ReadSymbolicLink(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path, 
		uapi::char_t* buffer, size_t length);

	static std::shared_ptr<Alias> ResolvePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path, int flags);

	// Remount
	//
	// Changes the options used for the file system mount
	//// virtual void Remount(uint32_t flags, const void* data) = 0;

	// Root
	//
	// Returns the root alias for the file system
	__declspec(property(get=getRoot)) AliasPtr Root;
	virtual AliasPtr getRoot(void) = 0;

	// Status
	//
	// Gets the file system status information	
	__declspec(property(get=getStatus)) uapi::statfs Status;
	virtual uapi::statfs getStatus(void) = 0;

private:

	FileSystem(const FileSystem&)=delete;
	FileSystem& operator=(const FileSystem&)=delete;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM_H_