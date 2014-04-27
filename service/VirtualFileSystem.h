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

#include <filesystem>
#include <memory>
#include <linux/stat.h>

#include "char_t.h"
#include "tstring.h"
#include "CompressedStreamReader.h"
#include "CpioArchive.h"
#include "Exception.h"
#include "File.h"
#include "VfsNode.h"
#include "VfsDirectoryNode.h"
#include "VfsFileNode.h"
#include "VfsResolveResult.h"
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

	// CreateDirectoryNode
	//
	// Creates a directory node
	//static VfsNodePtr CreateDirectoryNode(mode_t mode)
	//	{ return VfsNodePtr(new VfsDirectoryNode(mode)); }
	//static VfsNodePtr CreateDirectoryNode(mode_t mode, uid_t uid, gid_t gid)
	//	{ return VfsNodePtr(new VfsDirectoryNode(mode, uid, gid)); }

	// CreateFileNode
	//
	// Creates a file node
	//static VfsNodePtr CreateFileNode(mode_t mode)
	//	{ return VfsNodePtr(new VfsFileNode(mode)); }
	//static VfsNodePtr CreateFileNode(mode_t mode, uid_t uid, gid_t gid)
	//	{ return VfsNodePtr(new VfsFileNode(mode, uid, gid)); }
	//static VfsNodePtr CreateFileNode(mode_t mode, StreamReader& data)
	//	{ return VfsNodePtr(new VfsFileNode(mode, data)); }
	//static VfsNodePtr CreateFileNode(mode_t mode, uid_t uid, gid_t gid, StreamReader& data)
	//	{ return VfsNodePtr(new VfsFileNode(mode, uid, gid, data)); }

	// ResolvePath
	//
	// Resolves a string-based file system path
	VfsResolveResult ResolvePath(const char_t* path) { return ResolvePath(m_root, path); }
	VfsResolveResult ResolvePath(const VfsDirectoryNodePtr& root, const char_t* path);

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
	// Member Variables

	VfsDirectoryNodePtr		m_root;			// Root filesystem node
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALFILESYSTEM_H_
