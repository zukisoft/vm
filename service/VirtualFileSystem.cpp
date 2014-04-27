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

VirtualFileSystem::VirtualFileSystem() : m_root(new VfsDirectoryNode(m_root, S_IFDIR))
{
}

//-----------------------------------------------------------------------------
// VirtualFileSystem Destructor

VirtualFileSystem::~VirtualFileSystem()
{
}

//-----------------------------------------------------------------------------
// VirtualFileSystem::ResolvePath
//
// Resolves a string-based path against the virtual file system
//
// Arguments:
//
//	root		- Base node to begin the search from
//	path		- File system path string (ANSI)

VfsResolveResult VirtualFileSystem::ResolvePath(const VfsDirectoryNodePtr& root, const char_t* path)
{
	// Convert the C-style path into a <filesystem> path instance
	std::tr2::sys::path	pathstr(path);

	// If the path if rooted, ignore the provided node and use the actual root node
	VfsDirectoryNodePtr branch = (pathstr.has_root_directory()) ? m_root : root;

	// Pull out the desired alias string and remove it from the branch path
	std::string alias = pathstr.filename();
	pathstr = pathstr.relative_path().parent_path();

	// Iterate over the branch path first
	for(std::tr2::sys::path::iterator it = pathstr.begin(); it != pathstr.end(); it++) {

		// .
		// Special case indicating the current directory
		if(it->compare(".")) continue;

		// ..
		// Special case indicating the parent of the current directory
		else if(it->compare("..")) { branch = branch->Parent; continue; }

		// Get the next node in the branch path
		VfsNodePtr next = branch->GetAlias(it->c_str());
		if(next == nullptr) return VfsResolveResult(VfsResolveStatus::BranchNotFound);

		// Only directory and symbolic link nodes can be resolved as part of the branch
		switch(next->Mode & S_IFMT) {

			case S_IFDIR: 
				branch = std::dynamic_pointer_cast<VfsDirectoryNode>(next);
				if(branch == nullptr) return VfsResolveResult(VfsResolveStatus::BranchNotDirectory);
				break;
			
			case S_IFLNK: /* TODO */ break;

			default: return VfsResolveResult(VfsResolveStatus::BranchNotDirectory);
		};
	}

	// After the loop, branch is now set to the parent directory for the alias
	VfsNodePtr leaf = branch->GetAlias(alias.c_str());
	VfsResolveStatus status = (leaf == nullptr) ? VfsResolveStatus::FoundBranch : VfsResolveStatus::FoundLeaf;
	return VfsResolveResult(status, branch, leaf, alias);
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

		VfsResolveResult resolved = ResolvePath(m_root, file.Path);			// Resolve the parent path
		if(!resolved) { /* TODO: ERROR */ }
		
		// Depending on the type of node being enumerated, construct the appropriate object
		switch(file.Mode & S_IFMT) {

			// S_IFREG
			//
			// FoundBranch	--> Create a new file node
			// FoundNode	--> ERROR: The file already exists
			case S_IFREG:

				if(resolved.Status != VfsResolveStatus::FoundBranch) { /* TODO: ERROR - FILE EXISTS */ }
				resolved.Branch->AddAlias(resolved.Alias, std::make_shared<VfsFileNode>(file.Mode, file.UserId, file.GroupId, file.Data));
				break;

			// S_IFDIR
			//
			// FoundBranch	--> Create a new directory node
			// FoundNode	--> Directory exists, reset mode, uid and gid to match the new entry
			//
			case S_IFDIR:

				if(resolved.Status == VfsResolveStatus::FoundLeaf) break;  // <---- TODO: Change mode, uid and gid
				resolved.Branch->AddAlias(resolved.Alias, std::make_shared<VfsDirectoryNode>(resolved.Branch, file.Mode, file.UserId, file.GroupId));
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
