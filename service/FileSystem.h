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
// that can be used to more easily access file system functionality
//
// todo: Mount implementation needs bitmask<> treatment

class __declspec(novtable) FileSystem
{
public:

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
	class Path;
	class PathHandle;

	// FileSystem::MountFunction
	//
	// Function signature for a file system's Mount() implementation, which must be a public static method
	using MountFunction = std::function<std::shared_ptr<Mount>(const char_t* source, uint32_t flags, const void* data, size_t datalength)>;

	// FileSystem::MaxSymbolicLinks
	//
	// Constant indicating the maximum recursion depth of a path lookup
	static const int MaxSymbolicLinks = 40;

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

	// FileSystem::HandleAccess
	//
	// Access modes used with handle operations
	class HandleAccess final : public bitmask<HandleAccess, uint32_t, LINUX_O_RDONLY | LINUX_O_WRONLY | LINUX_O_RDWR>
	{
	public:

		using bitmask::bitmask;

		//-------------------------------------------------------------------------
		// Fields

		// ReadOnly (static)
		//
		// Indicates that the handle should be opened in read-only mode
		static const HandleAccess ReadOnly;

		// WriteOnly (static)
		//
		// Indicates that handle should be opened in write-only mode
		static const HandleAccess WriteOnly;

		// ReadWrite (static)
		//
		// Indicates that the handle should be opened in read/write mode
		static const HandleAccess ReadWrite;
	};

	// FileSystem::HandleFlags
	//
	// Flags used with handle operations
	class HandleFlags final : public bitmask<HandleFlags, uint32_t, LINUX_O_APPEND | LINUX_O_DIRECT | LINUX_O_DSYNC | LINUX_O_NOATIME | LINUX_O_SYNC | LINUX_O_TRUNC>
	{
	public:

		using bitmask::bitmask;

		//-------------------------------------------------------------------------
		// Fields

		// Append (static)
		//
		// Indicates that the handle should be opened in append mode
		static const HandleFlags Append;

		// DataSync (static)
		//
		// Indicates that write operations should enforce data integrity
		static const HandleFlags DataSync;

		// Direct (static)
		//
		// Indicates that the handle should not buffer input/output
		static const HandleFlags Direct;

		// NoAccessTime (static)
		//
		// Indicates that the handle should not update atime after a read operation
		static const HandleFlags NoAccessTime;

		// None (static)
		//
		// Indicates that no special handle flags are present
		static const HandleFlags None;

		// Sync (static)
		//
		// Indicates that write operations should enforce both file and data integrity
		static const HandleFlags Sync;

		// Truncate (static)
		//
		// Indicates that the underlying file should be truncated on open
		static const HandleFlags Truncate;
	};

	// FileSystem::LookupFlags
	//
	// Flags used with lookup operations
	class LookupFlags final : public bitmask<LookupFlags, uint32_t, LINUX_O_DIRECTORY | LINUX_O_NOFOLLOW>
	{
	public:

		using bitmask::bitmask;

		//-------------------------------------------------------------------------
		// Fields

		// Directory (static)
		//
		// Indicates that the target of a lookup must be a directory
		static const LookupFlags Directory;

		// NoFollow (static)
		//
		// Indicates that a symbolic link final path component should not be followed
		static const LookupFlags NoFollow;

		// None (static)
		//
		// Indicates that no special lookup flags are present
		static const LookupFlags None;
	};

	//
	// FILE SYSTEM INTERFACES
	//

	// FileSystem::Alias
	//
	// Interface that must be implemented by a file system alias.  An alias is
	// a name that is used to reference a file system node (basically a hard link)
	struct __declspec(novtable) Alias
	{
		// GetName
		//
		// Reads the name assigned to this alias
		virtual uapi::size_t GetName(char_t* buffer, size_t count) const = 0;

		// Name
		//
		// Gets the name assigned to this alias
		__declspec(property(get=getName)) std::string Name;
		virtual std::string getName(void) const = 0;

		// Node
		//
		// Gets the node to which this alias refers
		__declspec(property(get=getNode)) std::shared_ptr<FileSystem::Node> Node;
		virtual std::shared_ptr<FileSystem::Node> getNode(void) const = 0;
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
		virtual std::shared_ptr<Handle> Duplicate(void) const = 0;

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
		virtual void Sync(void) const = 0;

		// SyncData
		//
		// Synchronizes all data associated with the file to storage, not metadata
		virtual void SyncData(void) const = 0;

		// Write
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t Write(const void* buffer, uapi::size_t count) = 0;

		// WriteAt
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count) = 0;

		// Access
		//
		// Gets the access mode used when the handle was created
		__declspec(property(get=getAccess)) FileSystem::HandleAccess Access;
		virtual FileSystem::HandleAccess getAccess(void) const = 0;

		// Flags
		//
		// Gets the flags used when the handle was created
		__declspec(property(get=getFlags)) FileSystem::HandleFlags Flags;
		virtual FileSystem::HandleFlags getFlags(void) const = 0;
	};

	// FileSystem::Mount
	//
	// Interface that must be implemented by a file system mount.  A mount is a view
	// of a file system, normally associated with a specific namespace.
	struct __declspec(novtable) Mount
	{
		// Duplicate
		//
		// Duplicates this mount instance
		virtual std::shared_ptr<FileSystem::Mount> Duplicate(void) const = 0;

		// Remount
		//
		// Remounts this mount point with different flags and arguments
		virtual void Remount(uint32_t flags, const void* data, size_t datalen) = 0;

		// Stat
		//
		// Provides statistical information about the file system
		virtual void Stat(uapi::statfs* stats) const = 0;

		// Unmount
		//
		// Unmounts the file system
		virtual void Unmount(void) = 0;

		// Flags
		//
		// Gets the flags set on this mount, includes file system flags
		__declspec(property(get=getFlags)) uint32_t Flags;
		virtual uint32_t getFlags(void) const = 0;

		// Root
		//
		// Gets a reference to the root directory of the mount point
		__declspec(property(get=getRoot)) std::shared_ptr<FileSystem::Directory> Root;
		virtual std::shared_ptr<FileSystem::Directory> getRoot(void) const = 0;

		// Source
		//
		// Gets the device/name used as the source of the file system
		__declspec(property(get=getSource)) std::string Source;
		virtual std::string getSource(void) const = 0;
	};

	// FileSystem::Node
	//
	// Interface that must be implemented for a file system node object.  A node represents
	// any object (file, directory, socket, etc) that is a member of a file system
	struct __declspec(novtable) Node
	{
		// Open
		//
		// Creates a Handle instance against this node
		virtual std::shared_ptr<FileSystem::Handle> Open(std::shared_ptr<FileSystem::Mount> mount, FileSystem::HandleAccess access, FileSystem::HandleFlags flags) = 0;

		// SetOwnership
		//
		// Changes the ownership of this node
		virtual void SetOwnership(uapi::uid_t uid, uapi::gid_t gid) = 0;

		// SetPermissions
		//
		// Changes the permission flags for this node
		virtual void SetPermissions(uapi::mode_t mode) = 0;

		// Stat
		//
		// Provides statistical information about the node
		virtual void Stat(uapi::stat* stats) const = 0;

		// Type
		//
		// Gets the type of file system node being implemented
		__declspec(property(get=getType)) FileSystem::NodeType Type;
		virtual FileSystem::NodeType getType(void) const = 0;
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
		//// CreateCharacterDevice
		////
		//// Creates a new character device node as a child of this directory
		//virtual std::shared_ptr<FileSystem::Path> CreateCharacterDevice(std::shared_ptr<FileSystem::Mount> mount, const char_t* name, uapi::mode_t mode, uapi::dev_t device) = 0;

		// CreateDirectory
		//
		// Creates a new directory node as a child of this directory
		virtual std::shared_ptr<FileSystem::Alias> CreateDirectory(std::shared_ptr<FileSystem::Mount> mount, const char_t* name, uapi::mode_t mode) = 0;

		// CreateFile
		//
		// Creates a new regular file node as a child of this directory
		virtual std::shared_ptr<FileSystem::Alias> CreateFile(std::shared_ptr<FileSystem::Mount> mount, const char_t* name, uapi::mode_t mode) = 0;

		//// CreateSymbolicLink
		////
		//// Creates a new symbolic link as a child of this directory
		//virtual std::shared_ptr<FileSystem::Path> CreateSymbolicLink(std::shared_ptr<FileSystem::Mount> mount, const char_t* name, const char_t* target) = 0;

		// Lookup
		//
		// Looks up the alias associated with a child of this directory
		virtual std::shared_ptr<FileSystem::Alias> Lookup(std::shared_ptr<FileSystem::Mount> mount, const char_t* name) const = 0;
	};

	// FileSystem::File
	//
	// Specialization of Node for file system regular file objects
	struct __declspec(novtable) File : public Node
	{
		// OpenExec
		//
		// Creates an execute-only handle against this node; used only by the virtual machine
		virtual std::shared_ptr<FileSystem::Handle> OpenExec(std::shared_ptr<FileSystem::Mount> mount) const = 0;
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
		// GetTarget
		//
		// Reads the target of the symbolic link
		virtual uapi::size_t GetTarget(char_t* buffer, size_t count) const = 0;

		// Target
		//
		// Gets the symbolic link target string
		__declspec(property(get=getTarget)) std::string Target;
		virtual std::string getTarget(void) const = 0;
	};

	//
	// FILE SYSTEM CLASSES
	//

	// FileSystem::Path
	//
	// Path represents a fully resolved file system object
	class Path final
	{
	friend class FileSystem;
	public:

		// Copy Constructor
		//
		//Path(const Path& rhs);

		// Move Constructor
		//
		//Path(Path&& rhs);

		// Destructor
		//
		~Path()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Create (static)
		//
		// Creates a new Path instance from component parts
		static std::shared_ptr<Path> Create(std::shared_ptr<FileSystem::Alias> alias, std::shared_ptr<FileSystem::Mount> mount);

	private:

		Path(const Path&)=delete;
		Path& operator=(const Path&)=delete;
		//Path& operator=(Path&&)=delete;

		// Instance Constructors
		//
		Path(std::shared_ptr<FileSystem::Alias> alias, std::shared_ptr<FileSystem::Mount> mount);
		Path(std::shared_ptr<FileSystem::Path> parent, std::shared_ptr<FileSystem::Alias> alias, std::shared_ptr<FileSystem::Mount> mount);
		friend class std::_Ref_count_obj<Path>;

		//---------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<FileSystem::Path>		m_parent;	// Parent instance
		const std::shared_ptr<FileSystem::Alias>	m_alias;	// Alias reference
		const std::shared_ptr<FileSystem::Mount>	m_mount;	// Mount reference
	};

	// FileSystem::PathHandle
	//
	// Implementation of an O_PATH-based handle used by the File System API.  Individual
	// file systems do not need to implement any special handling when this flag has been
	// specified by a client call, no call to Node::Open() will take place, an instance
	// of this class will be used automatically
	class PathHandle final : public FileSystem::Handle
	{
	public:

		// Instance Constructor
		//
		PathHandle(std::shared_ptr<FileSystem::Node> node, FileSystem::HandleAccess access);

		// Destructor
		//
		~PathHandle()=default;

		//---------------------------------------------------------------------
		// FileSystem::Handle Implementation

		// Duplicate
		//
		// Creates a duplicate Handle instance
		virtual std::shared_ptr<FileSystem::Handle> Duplicate(void) const override;

		// Read
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t Read(void* buffer, uapi::size_t count) override;

		// ReadAt
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count) override;

		// Seek
		//
		// Changes the file position
		virtual uapi::loff_t Seek(uapi::loff_t offset, int whence) override;

		// Sync
		//
		// Synchronizes all metadata and data associated with the file to storage
		virtual void Sync(void) const override;

		// SyncData
		//
		// Synchronizes all data associated with the file to storage, not metadata
		virtual void SyncData(void) const override;

		// Write
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t Write(const void* buffer, uapi::size_t count) override;

		// WriteAt
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count) override;

		// getAccess
		//
		// Gets the handle access mode
		virtual FileSystem::HandleAccess getAccess(void) const override;

		// getFlags
		//
		// Gets the handle flags
		virtual FileSystem::HandleFlags getFlags(void) const override;

	private:

		PathHandle(const PathHandle&)=delete;
		PathHandle& operator=(const PathHandle&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<FileSystem::Node>	m_node;		// Referenced node
		const FileSystem::HandleAccess			m_access;	// Handle access mode
	};

	//
	// FILE SYSTEM API
	//

	// NOTE: These should use the weakly typed flags (LINUX_O_XXXX) to make it easier for the RPC layer to just
	// pass them in from the client application

	//static void CheckPermissions(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, 
	//	int flags, uapi::mode_t mode);

	//static void CreateCharacterDevice(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base,
	//	const char_t* path, uapi::mode_t mode, uapi::dev_t device);

	//static void CreateDirectory(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, uapi::mode_t mode);

	// CreateFile
	//
	// Creates a new regular file
	//static std::shared_ptr<Handle> CreateFile(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, 
	//	int flags, uapi::mode_t mode);

	//static void CreateSymbolicLink(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, const char_t* target);

	// GenerateFileSystemId
	//
	// Generates a unique file system identifier (fsid)
	static uapi::fsid_t GenerateFileSystemId(void);

	// GetAbsolutePath
	//
	// Determines the absolute file system path of an existing object
	///static void GetAbsolutePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& alias, char_t* path, size_t pathlen);

	// LookupPath
	//
	// Resolves a file system object as a FileSystem::Path instance
	static std::shared_ptr<FileSystem::Path> LookupPath(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
		std::shared_ptr<FileSystem::Path> current, const char_t* path, int flags);

	// OpenExecutable
	//
	// Opens an existing file system object as an executable
	static std::shared_ptr<FileSystem::Handle> OpenExecutable(std::shared_ptr<Namespace> ns, std::shared_ptr<Path> root, 
		std::shared_ptr<Path> current, const char_t* path);

	// OpenFile
	//
	// Opens or creates a file
	static std::shared_ptr<FileSystem::Handle> OpenFile(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
		std::shared_ptr<FileSystem::Path> current, const char_t* path, int flags, uapi::mode_t mode);

	// ReadSymbolicLink
	//
	// Reads the target path associated with a symbolic link
	static size_t ReadSymbolicLink(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
		std::shared_ptr<FileSystem::Path> current, const char_t* path, char_t* buffer, size_t length);

private:

	FileSystem()=delete;
	FileSystem(const FileSystem&)=delete;
	FileSystem& operator=(const FileSystem&)=delete;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// LookupPath
	//
	// Resolves a file system object as a FileSystem::Path instance
	static std::shared_ptr<FileSystem::Path> LookupPath(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
		std::shared_ptr<FileSystem::Path> current, const char_t* path, FileSystem::LookupFlags flags, int depth);
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM_H_