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

#include "stdafx.h"
#include "VirtualFileSystem.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VirtualFileSystem Constructor
//
// Arguments:
//
//	NONE

VirtualFileSystem::VirtualFileSystem()
{
	// Construct the root file system node
	m_root = new VfsDirectoryNode(S_IFDIR);		// <--- TODO: access mask
}

//-----------------------------------------------------------------------------
// VirtualFileSystem Destructor

VirtualFileSystem::~VirtualFileSystem()
{
	// Deletion of the root node will cascade delete the virtual file system
	delete m_root;
}

//-----------------------------------------------------------------------------
// VirtualFileSystem::LoadInitialFileSystem
//
// Loads an initramfs archive into the virtual file system
//
// Arguments:
//
//	path		- Path to the initramfs archive file

void VirtualFileSystem::LoadInitialFileSystem(const tchar_t* path)
{
	// Attempt to open the specified file read-only with sequential scan optimization
	std::unique_ptr<File> archive = File::OpenExisting(path, GENERIC_READ, FILE_SHARE_READ, FILE_FLAG_SEQUENTIAL_SCAN);

	// Decompress as necessary and iterate over all the files contained in the CPIO archive
	CpioArchive::EnumerateFiles(CompressedStreamReader::FromFile(archive), [&](const CpioFile& file) -> void {

		// Depending on the type of node being enumerated, construct the appropriate object
		switch(file.Mode & S_IFMT) {

			case S_IFREG:
				CreateFile(file);
				break;

			case S_IFDIR:
				CreateDirectory(file);
				break;

			case S_IFLNK:
				break;

			case S_IFCHR:
				break;

			case S_IFBLK:
				break;

			case S_IFIFO:
				break;

			case S_IFSOCK:
				break;

			default:
				break;
		}
	});
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
