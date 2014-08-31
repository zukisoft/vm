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

#ifndef __TEMPFILESYSTEM_H_
#define __TEMPFILESYSTEM_H_
#pragma once

#include <memory>
#include "LinuxException.h"
#include "FileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TempFileSystem
//
// 

class TempFileSystem : public FileSystem
{
public:

	// Destructor
	virtual ~TempFileSystem()=default;

private:

	TempFileSystem(const TempFileSystem&)=delete;
	TempFileSystem& operator=(const TempFileSystem&)=delete;

	// Constructor
	//
	TempFileSystem()=default;
	friend class std::_Ref_count_obj<TempFileSystem>;

	// TempFileSystem::MountPoint
	//
	// Internal state and metadata about the mounted file system, all file
	// system objects will hold a reference to this object
	class MountPoint
	{
	public:

		// Constructor / Destructor
		//
		MountPoint()=default;
		~MountPoint()=default;

		//---------------------------------------------------------------------
		// Member Functions

		//---------------------------------------------------------------------
		// Properties

	private:

		MountPoint(const MountPoint&)=delete;
		MountPoint& operator=(const MountPoint&)=delete;

		//---------------------------------------------------------------------
		// Member Variables
	};

	// TempFileSystem::Alias
	//
	// Specialization of FileSystem::Alias for a temp file system instance
	class Alias : public FileSystem::Alias
	{
	public:

		// Constructor / Destructor
		//
		Alias()=default;
		~Alias()=default;

	private:

		Alias(const Alias&)=delete;
		Alias& operator=(const Alias&)=delete;

		//---------------------------------------------------------------------
		// FileSystem::Alias Implementation

		// Mount
		//
		// Mounts/binds a foreign node to this alias, obscuring the previous node
		virtual void Mount(const FileSystem::NodePtr& node);

		// Unmount
		//
		// Unmounts/unbinds a node from this alias, revealing the previously bound node
		virtual void Unmount(void);

		// Name
		//
		// Gets the name associated with this alias
		__declspec(property(get=getName)) const tchar_t* Name;
		virtual const tchar_t* getName(void);

		// Node
		//
		// Gets the node instance that this alias references
		__declspec(property(get=getNode)) NodePtr Node;
		virtual NodePtr getNode(void);

		//---------------------------------------------------------------------
		// Private Member Functions

		//---------------------------------------------------------------------
		// Member Variables
	};

	// TempFileSystem::Node
	//
	// Specialization of FileSystem::Node for a temp file system instance 
	class Node : public FileSystem::Node
	{
	public:

		// Constructor / Destructor
		//
		Node()=default;
		~Node()=default;

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

		//---------------------------------------------------------------------
		// FileSystem::Node Implementation

		// CreateDirectory
		//
		// Creates a new directory node as a child of this node
		virtual void CreateDirectory(const tchar_t* name);

		// CreateFile
		//
		// Creates a new regular file node as a child of this node
		virtual HandlePtr CreateFile(const tchar_t* name, int flags);

		// CreateSymbolicLink
		//
		// Creates a new symbolic link as a child of this node
		virtual void CreateSymbolicLink(const tchar_t* name, const tchar_t* target);

		// OpenHandle
		//
		// Creates a FileSystem::Handle instance for this node
		virtual HandlePtr OpenHandle(int flags);

		// ResolvePath
		//
		// Resolves a relative path from this node to an Alias instance
		virtual AliasPtr ResolvePath(const tchar_t* path);

		// Index
		//
		// Gets the node index
		__declspec(property(get=getIndex)) uint64_t Index;
		virtual uint64_t getIndex(void);

		// Type
		//
		// Gets the node type
		__declspec(property(get=getType)) NodeType Type;
		virtual NodeType getType(void);

		//---------------------------------------------------------------------
		// Private Member Functions

		//---------------------------------------------------------------------
		// Member Variables
	};

	// Handle
	//
	// Specialization of FileSystem::Handle for temp file system instance
	class Handle : public FileSystem::Handle
	{
	public:

		Handle()=default;
		~Handle()=default;

	private:

		Handle(const Handle&)=delete;
		Handle& operator=(const Handle&)=delete;

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
		// Synchronizes all data associated with the file to storage, not metadata
		virtual void SyncData(void);

		// Write
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t Write(const void* buffer, uapi::size_t count);

		//---------------------------------------------------------------------
		// Private Member Functions

		//---------------------------------------------------------------------
		// Member Variables
	};

	//-------------------------------------------------------------------------
	// FileSystem Implementation

	// getRoot
	//
	// Accesses the root alias for the file system
	virtual AliasPtr getRoot(void);

	//---------------------------------------------------------------------
	// Private Member Functions

	//---------------------------------------------------------------------
	// Member Variables
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TEMPFILESYSTEM_H_