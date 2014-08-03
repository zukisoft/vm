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

#ifndef __TESTFILESYSTEM_H_
#define __TESTFILESYSTEM_H_
#pragma once

#include <atomic>
#include <concurrent_queue.h>
#include <concurrent_vector.h>
#include <filesystem>
#include <memory>
#include <stack>
#include <linux/stat.h>
#include "LinuxException.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)		// inline specifier cannot be used (friend)

struct __declspec(novtable) FileSystem
{
	// AliasPtr
	//
	// Alias for an std::shared_ptr<Alias>
	struct Alias;
	using AliasPtr = std::shared_ptr<Alias>;

	// FilePtr
	//
	// Alias for an std::shared_ptr<File>
	struct File;
	using FilePtr = std::shared_ptr<File>;

	// NodePtr
	//
	// Alias for an std::shared_ptr<Node>
	struct Node;
	using NodePtr = std::shared_ptr<Node>;

	// AliasState
	//
	// Defines the state of an alias
	enum class AliasState
	{
		Attached		= 0,		// Alias is attached to a Node
		Detached		= 1,		// Alias is not attached to a Node
	};

	// NodeType
	//
	// Strogly typed enumeration for the S_IFxxx inode type constants
	enum NodeType
	{
		BlockDevice			= LINUX_S_IFBLK,
		CharacterDevice		= LINUX_S_IFCHR,
		Directory			= LINUX_S_IFDIR,
		File				= LINUX_S_IFREG,
		Pipe				= LINUX_S_IFIFO,
		Socket				= LINUX_S_IFSOCK,
		SymbolicLink		= LINUX_S_IFLNK,
		Unknown				= 0,
	};

	// ModeToNodeType
	//
	// Converts a mode_t bitmask into a NodeType enumeration value
	static inline NodeType ModeToNodeType(const uapi::mode_t& mode)
	{
		// NodeType matches the bits from the S_IFMT mask
		return static_cast<NodeType>(mode & LINUX_S_IFMT);
	}

	// Alias
	//
	// Interface for a file system alias instance.  Similar in theory to the 
	// linux dentry, an alias is a named pointer to a file system node.
	//
	// Alias objects can optionally support attaching to multiple nodes to allow 
	// for mounting or otherwise masking an existing node; if this is not possible
	// for the file system, the PushMode() method should throw an exception if there
	// is already a node attached to the alias
	struct __declspec(novtable) Alias
	{
		// PopNode
		//
		// Pops a node from this alias to unmask an underlying node
		virtual NodePtr PopNode(void) = 0;

		// PushNode
		//
		// Pushes a node into this alias that masks any current node
		virtual void PushNode(const NodePtr& node) = 0;

		// Name
		//
		// Gets the name assigned to this alias instance
		__declspec(property(get=getName)) const tchar_t* Name;
		virtual const tchar_t* getName(void) = 0;

		// Node
		//
		// Gets a pointer to the underlying node for this alias
		//__declspec(property(get=getNode)) std::shared_ptr<Node> Node;  todo - name clash
		virtual NodePtr getNode(void) = 0;

		// Parent
		//
		// Gets the parent alias for this alias instance
		__declspec(property(get=getParent)) std::shared_ptr<Alias> Parent;
		virtual AliasPtr getParent(void) = 0;

		// State
		//
		// Gets the state (attached/detached) of this alias instance
		__declspec(property(get=getState)) AliasState State;
		virtual AliasState getState(void) = 0;
	};

	// File
	//
	// Interface for a file system file instance
	struct __declspec(novtable) File
	{
	};

	// Node
	//
	// Interface for a file system node instance
	struct __declspec(novtable) Node
	{
		__declspec(property(get=getIndex)) uint32_t Index;
		virtual uint32_t getIndex(void) = 0;

		// CreateNode
		//
		// Creates a child node using the specified Alias for the name; do not
		// link the node to the provided alias inside this method
		virtual NodePtr CreateNode(const AliasPtr& alias, uapi::mode_t mode) = 0;
	};

	//
	// FileSystem Members
	//

	__declspec(property(get=getRootNode)) NodePtr RootNode;
	virtual NodePtr getRootNode(void) = 0;
};

// Special alias to be incorporated into the service
// move to RootFileSystem.h/cpp
// Supports overmounting - yes

class VmRootAlias : public FileSystem::Alias
{
public:

	VmRootAlias()=default;
	~VmRootAlias()=default;

	virtual void PushNode(const std::shared_ptr<FileSystem::Node>& node)
	{
		std::lock_guard<std::mutex> critsec(m_nodelock);

		// Push the node into the stack, this will mask any existing node
		m_nodes.push(node);
	}

	virtual std::shared_ptr<FileSystem::Node> PopNode(void)
	{
		std::lock_guard<std::mutex> critsec(m_nodelock);

		if(m_nodes.empty()) return nullptr; 

		// Pull the top shared_ptr<> from the stack and pop it
		auto result = m_nodes.top();
		m_nodes.pop();
		return result;
	}

	// name is always blank for the root alias
	virtual const tchar_t* getName(void) { return _T(""); }

	virtual std::shared_ptr<FileSystem::Node> getNode(void) 
	{
		std::lock_guard<std::mutex> critsec(m_nodelock);

		// Return an empty shared_ptr<> if there is no attached node
		return (m_nodes.empty()) ? nullptr : m_nodes.top();
	}

	// Parent
	//
	// Gets the parent alias for this alias instance
	__declspec(property(get=getParent)) std::shared_ptr<FileSystem::Alias> Parent;
	virtual std::shared_ptr<FileSystem::Alias> getParent(void) { return nullptr; }

	// State
	//
	// Gets the state (attached/detached) of this alias instance
	__declspec(property(get=getState)) FileSystem::AliasState State;
	virtual FileSystem::AliasState getState(void) { 
		
		std::lock_guard<std::mutex> critsec(m_nodelock);
		return m_nodes.size() == 0 ? FileSystem::AliasState::Detached : FileSystem::AliasState::Attached; 
	}

private:

	VmRootAlias(const VmRootAlias&)=delete;
	VmRootAlias& operator=(const VmRootAlias&)=delete;

	std::stack<std::shared_ptr<FileSystem::Node>> m_nodes;
	std::mutex m_nodelock;
};

class TestFileSystem : public FileSystem
{
public:

	// Destructor
	//
	virtual ~TestFileSystem();

	// Mount
	//
	// Mounts the file system
	static std::unique_ptr<FileSystem> Mount(const tchar_t* device);

private:

	TestFileSystem(const TestFileSystem&)=delete;
	TestFileSystem& operator=(const TestFileSystem&)=delete;

	// Instance Constructor
	//
	TestFileSystem(const tchar_t* rootpath);
	friend std::unique_ptr<TestFileSystem> std::make_unique<TestFileSystem, const tchar_t*&>(const tchar_t*&);

	virtual std::shared_ptr<FileSystem::Node> getRootNode(void)
	{
		return m_root;
	}

	class Alias;
	class File;
	class Node;

	// Alias
	//
	// FileSystem::Alias implementation
	class Alias : public FileSystem::Alias
	{
	public:

		// Instance Constructors
		//
		Alias(const std::shared_ptr<FileSystem::Alias>& parent, const tchar_t* name) : m_parent(parent), m_name(name) {}
		Alias(const std::shared_ptr<FileSystem::Alias>& parent, const tchar_t* name, const std::shared_ptr<Node>& node) : m_parent(parent), m_name(name), m_node(node) {}

		// Destructor
		//
		virtual ~Alias()=default;

		// Name (FileSystem::Alias)
		//
		// Gets the name assigned to this alias instance
		__declspec(property(get=getName)) const tchar_t* Name;
		virtual const tchar_t* getName(void) const { return m_name.c_str(); }

		// Parent (FileSystem::Alias)
		//
		// Gets the parent alias for this alias instance
		__declspec(property(get=getParent)) std::shared_ptr<FileSystem::Alias> Parent;
		virtual std::shared_ptr<FileSystem::Alias> getParent(void) const { return nullptr; }

		// State (FileSystem::Alias)
		//
		// Gets the state (attached/detached) of this alias instance
		__declspec(property(get=getState)) FileSystem::AliasState State;
		virtual FileSystem::AliasState getState(void) const { return (m_node) ? AliasState::Attached : AliasState::Detached; }

	private:

		Alias(const Alias&)=delete;
		Alias& operator=(const Alias&)=delete;

		// m_name
		//
		// The name assigned to this alias
		std::tstring m_name;

		// m_node
		//
		// The node that is attached to this alias
		std::shared_ptr<Node> m_node;

		// m_parent
		//
		// Strong reference to this alias' parent alias
		std::shared_ptr<FileSystem::Alias> m_parent;
	};

	// File
	//
	// FileSystem::File implementation
	class File : public FileSystem::File
	{
	public:

		// Destructor
		//
		virtual ~File()=default;

	private:

		File(const File&)=delete;
		File& operator=(const File&)=delete;
	};

	// Node
	//
	// FileSystem::Node implementation
	// declspec(novtable) ?
	class Node : public FileSystem::Node
	{
	public:

		// Destructor
		//
		virtual ~Node();

		virtual uint32_t getIndex(void) { return static_cast<uint32_t>(m_index); }

	protected:

		// Instance Constructor
		//
		Node(TestFileSystem& fs, HANDLE handle, int32_t index) : m_fs(fs), m_handle(handle), m_index(index) {}

		// m_fs
		//
		// Reference to the parent file system object
		TestFileSystem& m_fs;

		// m_handle
		//
		// The underlying operating system object handle
		HANDLE m_handle = INVALID_HANDLE_VALUE;

		// m_index
		//
		// The allocated node index for this object
		int32_t m_index = 0;

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;
	};

	// DirectoryNode
	//
	// Specialization of Node for host directory objects
	class DirectoryNode : public Node
	{
	public:

		// Constructor / Destructor
		//
		DirectoryNode(TestFileSystem& fs, HANDLE handle, int32_t index);
		virtual ~DirectoryNode()=default;

		// CreateNode (FileSystem::Node)
		//
		// Creates a new node as a child of this node on the filesystem
		virtual NodePtr CreateNode(const AliasPtr& alias, uapi::mode_t mode)
		{
			// HostFileSystem only allows creation of directories, files and symbolic links
			NodeType type = FileSystem::ModeToNodeType(mode);
			if((type != NodeType::File) && (type != NodeType::Directory) && (type != NodeType::SymbolicLink)) throw LinuxException(LINUX_EINVAL);

			(alias);

			// need the path to the node, combine with alias name
			// create the object -- need to add SymbolicLinkNode
			// return the NodePtr
			return nullptr;
		}

		// TODO: open a child node
		// maybe rename as Get, or provide [] type access here
		// virtual NodePtr OpenNode(const AliasPtr& alias)

	private:

		DirectoryNode(const DirectoryNode&)=delete;
		DirectoryNode& operator=(const DirectoryNode&)=delete;
	};

	// FileNode
	//
	// Specialization of Node for host file objects
	class FileNode : public Node
	{
	public:

		// Constructor / Destructor
		//
		FileNode(TestFileSystem& fs, HANDLE handle, int32_t index);
		virtual ~FileNode()=default;

		// CreateNode (FileSystem::Node)
		//
		// Creates a new node as a child of this node on the filesystem
		virtual NodePtr CreateNode(const AliasPtr& alias, uapi::mode_t mode)
		{
			UNREFERENCED_PARAMETER(alias);
			UNREFERENCED_PARAMETER(mode);

			// The host file system does not allow files to have child nodes
			throw LinuxException(LINUX_EINVAL);
		}

	private:

		FileNode(const FileNode&)=delete;
		FileNode& operator=(const FileNode&)=delete;
	};

	// AllocateNodeIndex
	//
	// Allocates a node index from the pool
	int32_t AllocateNodeIndex(void);

	std::shared_ptr<DirectoryNode> CreateDirectoryNode(const tchar_t* path);

	std::shared_ptr<FileNode> CreateFileNode(const tchar_t* path);

	// ReleaseNodeIndex
	//
	// Releases a node index from the pool
	void RelaseNodeIndex(int32_t index);

	// m_nextindex
	//
	// Next sequential node index value
	std::atomic<int32_t> m_nextindex = 123;

	// m_root
	//
	// Strong reference to the root node
	std::shared_ptr<DirectoryNode> m_root;

	// m_spentindexes
	//
	// Queue used to recycle node indexes
	Concurrency::concurrent_queue<int32_t> m_spentindexes;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TESTFILESYSTEM_H_