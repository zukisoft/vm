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

#ifndef __VIRTUALFILESYSTEM_H_
#define __VIRTUALFILESYSTEM_H_
#pragma once

#include <memory>
#include <linux/stat.h>

#include "char_t.h"
#include "tstring.h"
#include "CompressedStreamReader.h"
#include "CpioArchive.h"
#include "Exception.h"
#include "File.h"
#include "LockedVfsNode.h"
#include "VfsNode.h"
#include "VfsDirectoryNode.h"
#include "VfsFileNode.h"
#include "Win32Exception.h"

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VirtualFileSystem
//
// Implements an in-memory virtual file system.  Unlike a full RAM disk, this
// implementation isn't meant to be general-purpose or accessible outside of 
// the process that created it.
//
// (This will ultimately require a lot of optimization -- make it work first)

class VirtualFileSystem
{
public:

	// Instance Constructors
	//
	VirtualFileSystem();

	// Destructor
	//
	~VirtualFileSystem();

	//-------------------------------------------------------------------------
	// Member Functions

	// CreateDirectory
	//
	// Creates a directory node
	static std::unique_ptr<LockedVfsNode> CreateDirectory(mode_t mode)
		{ return std::unique_ptr<LockedVfsNode>(new LockedVfsNode(new VfsDirectoryNode(mode))); }
	static std::unique_ptr<LockedVfsNode> CreateDirectory(mode_t mode, uid_t uid, gid_t gid)
		{ return std::unique_ptr<LockedVfsNode>(new LockedVfsNode(new VfsDirectoryNode(mode, uid, gid))); }

	// CreateFile
	//
	// Creates a file node
	static std::unique_ptr<LockedVfsNode> CreateFile(mode_t mode)
		{ return std::unique_ptr<LockedVfsNode>(new LockedVfsNode(new VfsFileNode(mode))); }
	static std::unique_ptr<LockedVfsNode> CreateFile(mode_t mode, uid_t uid, gid_t gid)
		{ return std::unique_ptr<LockedVfsNode>(new LockedVfsNode(new VfsFileNode(mode, uid, gid))); }
	static std::unique_ptr<LockedVfsNode> CreateFile(mode_t mode, StreamReader& data)
		{ return std::unique_ptr<LockedVfsNode>(new LockedVfsNode(new VfsFileNode(mode, data))); }
	static std::unique_ptr<LockedVfsNode> CreateFile(mode_t mode, uid_t uid, gid_t gid, StreamReader& data)
		{ return std::unique_ptr<LockedVfsNode>(new LockedVfsNode(new VfsFileNode(mode, uid, gid, data))); }

	// Find
	//
	// Locates a node in the virtual file system

	// need some form of result object:
	//	- FoundExact -- found the exact node
	//	- FoundParent -- found up through the parent
	//	- FoundAncestor	-- found an ancestor node
	//
	// need a flag to traverse links
	// need to addref() the node returned so it cannot be deleted
	//  (use the result class destructor to release?)
	//
	//void Find(const char_t* path);
	//void Find(const VfsNode* base, const char_t* path);

	// LoadInitialFileSystem
	//
	// Loads an initramfs archive into the virtual file system
	void LoadInitialFileSystem(const tchar_t* path);

	//-------------------------------------------------------------------------
	// Properties

private:

	VirtualFileSystem(const VirtualFileSystem&);
	VirtualFileSystem& operator=(const VirtualFileSystem&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// CreateDirectory
	//
	// Internal overloads to create a directory node
	static std::unique_ptr<LockedVfsNode> CreateDirectory(const CpioFile& cpiofile)
		{ return CreateDirectory(cpiofile.Mode, cpiofile.UserId, cpiofile.GroupId); }

	// CreateFile
	//
	// Internal overloads to create a file node
	static std::unique_ptr<LockedVfsNode> CreateFile(const CpioFile& cpiofile)
		{ return CreateFile(cpiofile.Mode, cpiofile.UserId, cpiofile.GroupId, cpiofile.Data); }

	//-------------------------------------------------------------------------
	// Member Variables

	VfsDirectoryNode*	m_root;				// Root filesystem node
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALFILESYSTEM_H_
