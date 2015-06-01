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

#ifndef __ROOTFILESYSTEM_H_
#define __ROOTFILESYSTEM_H_
#pragma once

#include <memory>
#include "FileSystem.h"
#include "LinuxException.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem
//
// RootFileSystem implements a virtual single directory node file system in which
// no additional objects can be created

class RootFileSystem : public FileSystem, public FileSystem::DirectoryBase, public std::enable_shared_from_this<RootFileSystem>
{
public:

	// Destructor
	//
	virtual ~RootFileSystem()=default;

	// Mount (static)
	//
	// Creates an instance of the file system
	static std::shared_ptr<FileSystem::Mount> Mount(const char_t* source, uint32_t flags, const void* data, size_t datalength);

	// CreateCharacterDevice (FileSystem::Directory)
	//
	// Creates a new character device node as a child of this node
	virtual void CreateCharacterDevice(const std::shared_ptr<Alias>& parent, const char_t* name, uapi::mode_t mode, uapi::dev_t device);

	// CreateDirectory (FileSystem::Directory)
	//
	// Creates a new directory node as a child of this node
	virtual void CreateDirectory(const std::shared_ptr<Alias>& parent, const char_t* name, uapi::mode_t mode);

	// CreateFile (FileSystem::Directory)
	//
	// Creates a new regular file node as a child of this node
	virtual std::shared_ptr<Handle> CreateFile(const std::shared_ptr<Alias>& parent, const char_t* name, int flags, uapi::mode_t mode);

	// CreateSymbolicLink (FileSystem::Directory)
	//
	// Creates a new symbolic link as a child of this node
	virtual void CreateSymbolicLink(const std::shared_ptr<Alias>& parent, const char_t* name, const char_t* target);

	// DemandPermission (FileSystem::Node)
	//
	// Demands read/write/execute permissions for the node (MAY_READ, MAY_WRITE, MAY_EXECUTE)
	virtual void DemandPermission(uapi::mode_t mode);

	// Open (FileSystem::Node)
	//
	// Creates a Handle instance against this node
	virtual std::shared_ptr<Handle> Open(const std::shared_ptr<Alias>& alias, int flags);

	// Lookup (FileSystem::Node)
	//
	// Resolves a relative path to an alias from this node
	virtual std::shared_ptr<Alias> Lookup(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& current, 
		const char_t* path, int flags, int* symlinks);
		
	// Stat (FileSystem)
	//
	// Provides statistical information about the file system
	virtual void Stat(uapi::statfs* stats);

	// Stat (FileSystem::Node)
	//
	// Provides statistical information about the node
	virtual void Stat(uapi::stat* stats);

	// getFileSystem (FileSystem::Node)
	//
	// Gets a reference to this node's parent file system instance
	virtual std::shared_ptr<::FileSystem> getFileSystem(void);

	// getRoot (FileSystem)
	//
	// Gets a reference to the root file system node
	virtual std::shared_ptr<Node> getRoot(void);

	// getSource (FileSystem)
	//
	// Gets the device/name used as the source of the file system
	virtual const char_t* getSource(void);

private:

	RootFileSystem(const RootFileSystem&)=delete;
	RootFileSystem& operator=(const RootFileSystem&)=delete;

	//// Mount
	////
	//// FileSystem::Mount implementation
	//class Mount : public FileSystem::Mount
	//{
	//public:

	//	// Constructor / Destructor
	//	//
	//	Mount(const std::shared_ptr<RootFileSystem>& fs);
	//	~Mount()=default;

	//private:

	//	Mount(const Mount&)=delete;
	//	Mount& operator=(const Mount&)=delete;
	//
	//	// Mount Methods
	//	//
	//	virtual std::shared_ptr<::Mount> Duplicate(void);
	//	virtual void Remount(uint32_t flags, const void* data, size_t datalen);

	//	// Mount Properties
	//	//
	//	virtual std::shared_ptr<FileSystem2> getFileSystem(void);

	//	// Member Variables
	//	//
	//	std::shared_ptr<RootFileSystem> m_fs;
	//};

	// Instance Constructor
	//
	RootFileSystem(const char_t* source);
	friend class std::_Ref_count_obj<RootFileSystem>;

	//-------------------------------------------------------------------------
	// Member Variables

	std::string				m_source;		// Source device name
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ROOTFILESYSTEM_H_