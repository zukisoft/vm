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

#ifndef __FILESYSTEM_H_
#define __FILESYSTEM_H_
#pragma once

#include <atomic>
#include <stack>
#include <vector>
#include <linux/stat.h>
#include <linux/types.h>
#include "LinuxException.h"

#pragma warning(push, 4)

// todo: document

struct __declspec(novtable) FileSystem
{
	// DirectoryEntryPtr
	//
	// s for an std::shared_ptr<DirectoryEntry>
	class DirectoryEntry;
	using DirectoryEntryPtr = std::shared_ptr<DirectoryEntry>;

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

	// need typedef for Mount(const tchar_t* device, uint32_t flags, const void* data)
	// need table type for mountable file systems -> Mount() function pointers

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

	// DirectoryEntry
	//
	// todo: could create static buckets for objects/locks, consider hashing this
	// and linking to a critical section; would save a lot of memory at a 
	// minimal performance impact
	class DirectoryEntry : public std::enable_shared_from_this<DirectoryEntry>
	{
	public:

		~DirectoryEntry()=default;

		// need a static create method

		// need something to iterate children, pass in a lambda to allow
		// for thread safety

		// pushes a node; sets mountpoint flag
		// may need an atomic counter for this to clear flag
		void Mount(const NodePtr& node)
		{
			return PushNode(node);
		}

		// pops a node; optionally clears mountpoint flag
		// may need atomic counter for this
		void Unmount(void)
		{
			PopNode();
		}

		// determines if this dentry is acting as a mount point; this can
		// be used to prevent the entry from ever dying off? that would be bad
		bool getMountPoint(void) const { return m_mounts > 0; }

		__declspec(property(get=getName)) const tchar_t* Name;
		const tchar_t* getName(void) const 
		{ 
			// how to deal with thread-safety during renames; could make this const
			// and force creation of a new directory entry instead, how would that
			// work if the node is attached
			return m_name.c_str(); 
		}

		std::shared_ptr<Node> getNode(void)
		{
			std::lock_guard<std::recursive_mutex> critsec(m_lock);
			return (m_nodes.empty()) ? nullptr : m_nodes.top();
		}

		std::shared_ptr<DirectoryEntry> getParent(void) const 
		{ 
			return m_parent; 
		}

		// operations
		// todo: could be useful when Find() gets a non-existent leaf node, and can call from tchar_t* version?
		// DirectoryEntryPtr CreateDirectory(const DirectoryEntryPtr& dentry, uapi::mode_t mode);
		DirectoryEntryPtr CreateDirectory(const tchar_t* name, uapi::mode_t mode);
		DirectoryEntryPtr CreateSymbolicLink(const tchar_t* name, const tchar_t* target);

	private:

		DirectoryEntry(const DirectoryEntry&)=delete;
		DirectoryEntry& operator=(const DirectoryEntry&)=delete;

		// Instance Constructors
		//
		DirectoryEntry(const tchar_t* name) : DirectoryEntry(name, nullptr, nullptr) {}
		DirectoryEntry(const tchar_t* name, const DirectoryEntryPtr& parent) : DirectoryEntry(name, parent, nullptr) {}
		DirectoryEntry(const tchar_t* name, const DirectoryEntryPtr& parent, const NodePtr& node);
		friend class std::_Ref_count_obj<DirectoryEntry>;

		// adds a node
		void PushNode(const NodePtr& node);

		// removes a node - throws if empty
		void PopNode(void);

		// m_lock
		//
		// Synchronization object
		std::recursive_mutex m_lock;

		// m_mounts
		//
		// The number of nodes in the stack that are mount points
		std::atomic<uint8_t> m_mounts = 0;

		// name
		// can this be const; how to deal with renames
		// what happens when we are a parent object, can't clone, perhaps could swap
		std::tstring m_name;

		// strong reference to parent, this is why an entry must
		// be able to be renamed in-place
		std::shared_ptr<DirectoryEntry> m_parent;

		// strong references to all nodes
		// todo: thread-safe
		std::stack<std::shared_ptr<Node>> m_nodes;

		// weak references to all known children
		// todo: thread-safe
		std::vector<std::weak_ptr<DirectoryEntry>> m_children;
	};

	// File
	//
	// todo: document when done
	struct __declspec(novtable) File
	{
	};

	// Node
	//
	// todo: document when done
	struct __declspec(novtable) Node
	{
		// CreateDirectory
		//
		// Creates a directory node as a child of this node on the file system
		virtual NodePtr CreateDirectory(const DirectoryEntryPtr& dentry, uapi::mode_t mode) = 0;

		// CreateSymbolicLink
		//
		// Creates a symbolic link node as a child of this node on the file system
		virtual NodePtr CreateSymbolicLink(const DirectoryEntryPtr& dentry, const tchar_t* target) = 0;

		// Index
		//
		// Gets the node index
		__declspec(property(get=getIndex)) uint32_t Index;
		virtual uint32_t getIndex(void) = 0;

		// Type
		//
		// Gets the node type
		__declspec(property(get=getType)) NodeType Type;
		virtual NodeType getType(void) = 0;
	};

	//
	// FileSystem Members
	//

	static DirectoryEntryPtr s_root;
};

__declspec(selectany) 
FileSystem::DirectoryEntryPtr FileSystem::s_root = std::make_shared<DirectoryEntry>(_T("TEST"));

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM_H_