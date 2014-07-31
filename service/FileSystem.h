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

#include <memory>
#include <concurrent_vector.h>
#include <linux/fs.h>
#include <linux/types.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Class FileSystem

class FileSystem
{
public:

	// #define FS_REQUIRES_DEV         1 
	// #define FS_BINARY_MOUNTDATA     2
	// #define FS_HAS_SUBTYPE          4
	// #define FS_USERNS_MOUNT         8       /* Can be mounted by userns root */
	// #define FS_USERNS_DEV_MOUNT     16 /* A userns mount does not imply MNT_NODEV */
	// #define FS_RENAME_DOES_D_MOVE   32768   /* FS will handle d_move() during rename() internally. */
	enum class Flags
	{
	};

	// Destructor
	//
	virtual ~FileSystem()=default;

	class Node;
	class DirectoryEntry;
	class File;

	// DIRENTRY ---strong---> NODE      positive dentry; keeps node alive
	// DIRENTRY ---strong---> <<NULL>>  negative dentry
	// AttachNode() -- makes positive
	// DetachNode() -- makes negative
	// DIRENTRY() --> negative at construct
	// DIRENTRY(Node) --> positive at construct

	// NODE ---weak---> DIRENTRY(S) can be none; will nullptr when they die
	// NODE ---weak---> FILE(S) can be none; will nullptr when they die

	// FILE ---strong---> NODE      keeps node alive
	// FILE ---strong---> DIRENTRY  keeps dentry alive


	class __declspec(novtable) DirectoryEntry : 
		public std::enable_shared_from_this<FileSystem::DirectoryEntry>
	{
	public:

		// Destructor
		//
		virtual ~DirectoryEntry()=default;

		// Open
		//
		// 
		std::shared_ptr<FileSystem::File> OpenFile(void)
		{
			if(!m_node) throw std::exception("TODO: new exception class");
			return m_node->OpenFile(shared_from_this());
		}

		// Node
		//
		// Gets/Sets the Node instance pointed to by this DirectoryEntry
		__declspec(property(get=getNode, put=putNode)) std::shared_ptr<FileSystem::Node> Node;
		std::shared_ptr<FileSystem::Node> getNode(void) const { return m_node; }
		void putNode(const std::shared_ptr<FileSystem::Node>& value) { m_node = value; }

	protected:

		// negative
		DirectoryEntry() {}

		// positive
		DirectoryEntry(const std::shared_ptr<FileSystem::Node>& node) : m_node(node)
		{
			_ASSERTE(node);						// Node must be alive
		}

	private:

		DirectoryEntry(const DirectoryEntry&)=delete;
		DirectoryEntry& operator=(const DirectoryEntry&)=delete;

		// strong reference to node
		std::shared_ptr<FileSystem::Node> m_node;
	};

	class __declspec(novtable) File : 
		public std::enable_shared_from_this<FileSystem::File>
	{
	public:

		// Destructor
		//
		virtual ~File()=default;

		// Read
		//
		// Synchronously reads data from the file
		virtual uapi::size_t Read(void* buffer, uapi::size_t count, uapi::loff_t pos) = 0;

		// Seek
		//
		// Sets the file pointer
		virtual uapi::loff_t Seek(uapi::loff_t offset, int origin) = 0;

		// Sync
		//
		// Flushes any buffered data to the underlying storage medium	
		virtual void Sync(void) = 0;

		// Write
		//
		// Synchronously writes data to the file
		virtual uapi::size_t Write(void* buffer, uapi::size_t count, uapi::loff_t pos) = 0;
	
	protected:

		// file needs a dentry and a node
		File(const std::shared_ptr<FileSystem::DirectoryEntry>& dentry, const std::shared_ptr<FileSystem::Node>& node) :
			m_dentry(dentry), m_node(node)
		{
			_ASSERTE(dentry && node);			// Both objects must be alive
		}


	private:

		File(const File&)=delete;
		File& operator=(const File&)=delete;

		// strong references to dentry and node
		std::shared_ptr<FileSystem::DirectoryEntry> m_dentry;
		std::shared_ptr<FileSystem::Node> m_node;
	};

	class __declspec(novtable) Node : 
		public std::enable_shared_from_this<FileSystem::Node>
	{
	public:

		// Destructor
		//
		virtual ~Node()=default;

		// Creates File against this Node
		// 
		virtual std::shared_ptr<FileSystem::File> OpenFile(const std::shared_ptr<FileSystem::DirectoryEntry>& dentry) = 0;
		//{
		//	//// this can't be here, must be in derived class
		//	//std::shared_ptr<FileSystem::File> result = std::make_shared<File>(dentry, shared_from_this());
		//	//m_files.push_back(result);
		//	//return result;
		//	return nullptr;
		//}

	protected:

		// detached node
		Node() {}

		// attached node
		Node(const std::shared_ptr<FileSystem::DirectoryEntry>& dentry)
		{
			m_dentries.push_back(dentry);
		}

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

		// m_dentries
		//
		// Collection of weak references to DirectoryEntry instances that point to this node
		Concurrency::concurrent_vector<std::weak_ptr<FileSystem::DirectoryEntry>> m_dentries;

		// m_files
		//
		// Collection of weak referecnes to File instance that point to this node
		Concurrency::concurrent_vector<std::weak_ptr<FileSystem::File>> m_files;
	};

protected:

	FileSystem()=default;

private:

	FileSystem(const FileSystem&)=delete;
	FileSystem& operator=(const FileSystem&)=delete;
	
	// static methods/member variables
	//
	// --> const char *name;
	// --> int fs_flags;
	
	// becomes Mount() in derived class
	// --> struct dentry *(*mount) (struct file_system_type *, int flags, const char * devname, void * data);
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM_H_