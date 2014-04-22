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
	VirtualFileSystem() : VirtualFileSystem(nullptr) {}
	VirtualFileSystem(const tchar_t* tempdir);

	// Destructor
	//
	~VirtualFileSystem();

	//-------------------------------------------------------------------------
	// Member Functions

	//VfsNode* CreateFileNode(

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

	VfsNode* CreateFileNode(const CpioFile& cpiofile);

	// GetTemporaryDirectory
	//
	// Generates a unique temporary directory name for the current process
	static std::tstring GetTemporaryDirectory(void);

	// GetUuid
	//
	// Generates a UUID string
	static std::tstring GetUuid(void);

	//-------------------------------------------------------------------------
	// Member Variables

	VfsDirectoryNode*	m_root;				// Root filesystem node
	std::tstring		m_tempdir;			// Temporary directory
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALFILESYSTEM_H_
