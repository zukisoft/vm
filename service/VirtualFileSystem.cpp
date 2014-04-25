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

VirtualFileSystem::VirtualFileSystem() : m_root(new VfsDirectoryNode(S_IFDIR))
{
}

//-----------------------------------------------------------------------------
// VirtualFileSystem Destructor

VirtualFileSystem::~VirtualFileSystem()
{
}

//-----------------------------------------------------------------------------
// VirtualFileSystem::Find
//
// Executes a path search against the virtual file system
//
// Arguments:
//
//	base		- Base node to begin the search from
//	path		- File system path string (ANSI)

VfsSearchResult VirtualFileSystem::Find(const VfsNodePtr& base, const char_t* path)
{
	// Convert the C-style path into a <filesystem> path instance
	std::tr2::sys::path	pathstr(path);

	// If the path contains the root directory, ignore base and start at m_root.
	VfsNodePtr current((pathstr.has_root_directory()) ? m_root : base);

	// Remove the root directory from the path string
	pathstr = pathstr.relative_path();

	// Iterate over the path components to traverse the tree
	for(auto it = pathstr.begin(); it != pathstr.end(); it++) {

		// TODO: Interesting stuff here
	}

	// TODO: this is a dummy value
	return VfsSearchResult(VfsSearch::FoundParent, VfsNodePtr(current));
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

		// All initramfs paths are based on the root file system node
		VfsSearchResult search = Find(m_root, file.Path);

		// TODO: Search result should always be parent when processing the initramfs,
		// otherwise that will be an error

		// Depending on the type of node being enumerated, construct the appropriate object
		switch(file.Mode & S_IFMT) {

			case S_IFREG:
				CreateFileNode(file);
				break;

			case S_IFDIR:
				CreateDirectoryNode(file);
				break;

			case S_IFLNK:
				//_RPTF0(_CRT_ASSERT, "initramfs: S_IFLNK not implemented yet");
				break;

			case S_IFCHR:
				_RPTF0(_CRT_ASSERT, "initramfs: S_IFCHR not implemented yet");
				break;

			case S_IFBLK:
				_RPTF0(_CRT_ASSERT, "initramfs: S_IFBLK not implemented yet");
				break;

			case S_IFIFO:
				_RPTF0(_CRT_ASSERT, "initramfs: S_IFIFO not implemented yet");
				break;

			case S_IFSOCK:
				_RPTF0(_CRT_ASSERT, "initramfs: S_IFSOCK not implemented yet");
				break;

			default:
				break;
		}
	});
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
