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
#include "Directory.h"
#include "FileSystem2.h"
#include "LinuxException.h"
#include "Mount.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem
//
// RootFileSystem implements a virtual single directory node file system in which
// no additional objects can be created.  Typically used only to provide a default
// virtual file system root node until a proper file system can be mounted
//
// IMPLEMENTS:
//
//	FileSystem
//	Mount
//	Directory

class RootFileSystem : public FileSystem2, public Directory, public std::enable_shared_from_this<RootFileSystem>
{
public:

	// Destructor
	//
	virtual ~RootFileSystem()=default;

	// Mount (static)
	//
	// Creates an instance of the file system
	static std::shared_ptr<::Mount> Mount(const char_t* const source, uint32_t flags, const void* data, size_t datalen);

private:

	RootFileSystem(const RootFileSystem&)=delete;
	RootFileSystem& operator=(const RootFileSystem&)=delete;

	// Mount
	//
	// Private implementation of the Mount interface
	class Mount : public ::Mount
	{
	public:

		// Constructor / Destructor
		//
		Mount(const std::shared_ptr<RootFileSystem>& fs);
		~Mount()=default;

	private:

		Mount(const Mount&)=delete;
		Mount& operator=(const Mount&)=delete;
	
		// Mount Methods
		//
		virtual std::shared_ptr<::Mount> Duplicate(void);
		virtual void Remount(uint32_t flags, const void* data, size_t datalen);

		// Mount Properties
		//
		virtual std::shared_ptr<FileSystem2> getFileSystem(void);

		// Member Variables
		//
		std::shared_ptr<RootFileSystem> m_fs;
	};

	// Instance Constructor
	//
	RootFileSystem(const char_t* source);
	friend class std::_Ref_count_obj<RootFileSystem>;

	// FileSystem Methods
	//
	virtual void Stat(uapi::statfs * stats);

	// FileSystem Properties
	//
	virtual std::shared_ptr<Node> getRoot(void);
	virtual const char_t* getSource(void);

	// Node Methods
	//
	virtual void DemandPermission(uapi::mode_t mode);
	virtual std::shared_ptr<Handle> Open(const std::shared_ptr<Alias>& alias, int flags);
	virtual std::shared_ptr<Alias> Lookup(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& current, const char_t* path, int flags, int* symlinks);
	virtual void Stat(uapi::stat * stats);

	// Node Properties
	//
	virtual std::shared_ptr<FileSystem2> getFileSystem(void);
	virtual NodeType getType(void);
	
	// Directory Methods
	//
	virtual void CreateCharacterDevice(const std::shared_ptr<Alias>& parent, const char_t* name, uapi::mode_t mode, uapi::dev_t device);
	virtual void CreateDirectory(const std::shared_ptr<Alias>& parent, const char_t* name, uapi::mode_t mode);
	virtual std::shared_ptr<Handle> CreateFile(const std::shared_ptr<Alias>& parent, const char_t* name, int flags, uapi::mode_t mode);
	virtual void CreateSymbolicLink(const std::shared_ptr<Alias>& parent, const char_t* name, const char_t* target);

	//-------------------------------------------------------------------------
	// Member Variables

	std::string				m_source;			// Source device
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ROOTFILESYSTEM_H_