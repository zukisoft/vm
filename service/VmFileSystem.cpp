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
#include "VmFileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// VmFileSystem Constructor
//
// Arguments:
//
//	rootfs		- Mounted FileSystem instance to serve as the root

VmFileSystem::VmFileSystem(const FileSystemPtr& rootfs)
{
	_ASSERTE(rootfs);
	m_rootfs = rootfs;
	m_rootdir = DirectoryEntry::Create(_T(""), nullptr, rootfs->RootNode);
}

void VmFileSystem::CreateDirectory(const tchar_t* path, uapi::mode_t mode)
{
	(mode);
	(path);

	DirectoryEntryPtr dentry = ResolvePath(path);
	// dentry == null --> cannot resolve path
	// dentry with a node --> found
	// dentry without a node --> not found, parent is valid
}

// Find
//
// nullptr --> path could not be resolved
// dentry with a node --> path was fully resolved
// dentry without a node --> path was resolved to parent only
DirectoryEntryPtr VmFileSystem::ResolvePath(const DirectoryEntryPtr& base, const tchar_t* path)
{
	tpath pathstr(path);

	// Determine the starting point for the search, if the provided path is rooted, use
	// the master root node, otherwise begin the search at the provided directory entry
	DirectoryEntryPtr branch = (pathstr.has_root_directory() ? m_rootdir : base);

	// Pull out the desired leaf name string and remove it from the branch path
	std::tstring leafstr = pathstr.filename();
	pathstr = pathstr.relative_path().parent_path();

	// Iterate over the branch path first
	for(tpath::iterator it = pathstr.begin(); it != pathstr.end(); it++) {

		// .
		// Special case indicating the current directory
		if(it->compare(_T(".")) == 0) continue;

		// ..
		// Special case indicating the parent of the current directory
		else if(it->compare(_T("..")) == 0) { 
		
			// Move up to the branch's parent, if one exists.  If there is no
			// parent this is the root node so behave the same as "." would
			if(branch->Parent != nullptr) branch = branch->Parent;
			continue; 
		}

		// CONTINUE HERE - more code in github
		//// Get the next node in the branch path
		//VfsNodePtr next = branch->GetAlias(it->c_str());
		//if(next == nullptr) return VfsResolveResult(VfsResolveStatus::BranchNotFound);

		//// Only directory and symbolic link nodes can be resolved as part of the branch
		//uapi::mode_t nodetype = next->Mode;
		//
		//if(uapi::S_ISDIR(nodetype)) {

		//	branch = std::dynamic_pointer_cast<VfsDirectoryNode>(next);
		//	if(branch == nullptr) return VfsResolveResult(VfsResolveStatus::BranchNotDirectory);
		//}

		//else if(uapi::S_ISLNK(nodetype)) {

		//	VfsSymbolicLinkNodePtr link = std::dynamic_pointer_cast<VfsSymbolicLinkNode>(next);
		//	if(link == nullptr) return VfsResolveResult(VfsResolveStatus::BranchNotDirectory);

		//	// Chase the symbolic link (this isn't optional for branch paths)
		//	VfsResolveResult chase = ResolvePath(branch, link->Target, true, level);
		//	if(!chase) return chase;

		//	// The symbolic link must ultimately end in resolution of a directory
		//	branch = std::dynamic_pointer_cast<VfsDirectoryNode>(chase.Leaf);
		//	if(branch == nullptr) return VfsResolveResult(VfsResolveStatus::BranchNotDirectory);
		//}
	}

	return nullptr;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
