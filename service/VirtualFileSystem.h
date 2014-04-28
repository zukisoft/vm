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
#include "VfsSymbolicLinkNode.h"
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

	// Instance Constructor
	//
	VirtualFileSystem() : m_root(std::make_shared<VfsDirectoryNode>(nullptr, ROOT_DIRECTORY_MODE)) {}

	//-------------------------------------------------------------------------
	// Member Functions

	// ResolvePath
	//
	// Resolves a string-based file system path
	VfsResolveResult ResolvePath(const char_t* path) { return ResolvePath(m_root, path, true, 0); }
	VfsResolveResult ResolvePath(const char_t* path, bool followlink) { return ResolvePath(m_root, path, followlink, 0); }
	VfsResolveResult ResolvePath(const VfsDirectoryNodePtr& root, const char_t* path) { return ResolvePath(root, path, true, 0); }
	VfsResolveResult ResolvePath(const VfsDirectoryNodePtr& root, const char_t* path, bool followlink) { return ResolvePath(root, path, followlink, 0); }

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
	// Private Constants

	// MAX_PATH_RECURSION
	//
	// Maximum number of recursive path resolution calls that can be made
	const uint32_t MAX_PATH_RECURSION = 40;

	// ROOT_DIRECTORY_MODE
	//
	// Defines the mode_t value for the root directory (S_IFDIR | 0755)
	const mode_t ROOT_DIRECTORY_MODE = (S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ResolvePath
	//
	// Private version of ResolvePath that tracks recursion
	VfsResolveResult ResolvePath(const VfsDirectoryNodePtr& root, const char_t* path, bool followlink, uint32_t level);

	//-------------------------------------------------------------------------
	// Member Variables

	VfsDirectoryNodePtr		m_root;			// Root filesystem node
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALFILESYSTEM_H_
