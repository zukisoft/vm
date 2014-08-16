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

#ifndef __FILESYSTEM_H_
#define __FILESYSTEM_H_
#pragma once

#include <linux/stat.h>
#include <linux/types.h>

#pragma warning(push, 4)

// todo: document

struct FileSystem;
using FileSystemPtr = std::shared_ptr<FileSystem>;

//-----------------------------------------------------------------------------
// FileSystem
//
// FileSystem represents the interface that must be implemented by all file system
// classes.
//
// todo: more words

struct __declspec(novtable) FileSystem
{
	// FilePtr
	//
	// Alias for an std::shared_ptr<File>
	struct File;
	using FilePtr = std::shared_ptr<File>;

	// NodePtr
	//
	// Alias for an std::shared_ptr<Node>
	struct Node;
	using NodePtr = std::shared_ptr<Node>;

	// need typedef for Mount(const tchar_t* device, uint32_t flags, const void* data)
	// need table type for mountable file systems -> Mount() function pointers

	// NodeType
	//
	// Strogly typed enumeration for the S_IFxxx inode type constants
	enum NodeType
	{
		BlockDevice			= LINUX_S_IFBLK,
		CharacterDevice		= LINUX_S_IFCHR,
		Directory			= LINUX_S_IFDIR,
		File				= LINUX_S_IFREG,
		Pipe				= LINUX_S_IFIFO,
		Socket				= LINUX_S_IFSOCK,
		SymbolicLink		= LINUX_S_IFLNK,
		Unknown				= 0,
	};

	// File
	//
	// todo: document when done
	struct __declspec(novtable) File
	{
	};

	// Node
	//
	// todo: document when done
	struct __declspec(novtable) Node
	{
		// CreateDirectory
		//
		// Creates a directory node as a child of this node on the file system
		virtual NodePtr CreateDirectory(const tchar_t* name, uapi::mode_t mode) = 0;

		// CreateSymbolicLink
		//
		// Creates a symbolic link node as a child of this node on the file system
		virtual NodePtr CreateSymbolicLink(const tchar_t* name, const tchar_t* target) = 0;

		// Index
		//
		// Gets the node index
		__declspec(property(get=getIndex)) uint32_t Index;
		virtual uint32_t getIndex(void) = 0;

		// Type
		//
		// Gets the node type
		__declspec(property(get=getType)) NodeType Type;
		virtual NodeType getType(void) = 0;
	};

	//
	// FileSystem Members
	//

	// RootNode
	//
	// Returns the root node for the file system
	__declspec(property(get=getRootNode)) NodePtr RootNode;
	virtual NodePtr getRootNode(void) = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM_H_