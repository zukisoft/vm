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
// todo: words

struct __declspec(novtable) FileSystem
{
	// AliasPtr
	//
	// Alias for an std::shared_ptr<Alias>
	struct Alias;
	using AliasPtr = std::shared_ptr<Alias>;

	// NodePtr
	//
	// Alias for an std::shared_ptr<Node>
	struct Node;
	using NodePtr = std::shared_ptr<Node>;

	// ViewPtr
	//
	// Alias for an std::shared_ptr<View>
	struct View;
	using ViewPtr = std::shared_ptr<View>;

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

	// NODE_INDEX_ROOT
	//
	// Constant indicating the node index for a file system root node
	static const uint32_t NODE_INDEX_ROOT = 2;

	// NODE_INDEX_LOSTANDFOUND
	//
	// Constant indicating the node index for a lost+found directory node
	static const uint32_t NODE_INDEX_LOSTANDFOUND = 3;

	// NODE_INDEX_FIRSTDYNAMIC
	//
	// Constant indicating the first dynamic node index that should be used
	static const uint32_t NODE_INDEX_FIRSTDYNAMIC = 4;

	// Alias
	//
	// todo: document when done
	struct __declspec(novtable) Alias
	{
		// Name
		//
		// Gets the name associated with this alias
		__declspec(property(get=getName)) const tchar_t* Name;
		virtual const tchar_t* getName(void) = 0;

		// Node
		//
		// Gets the node instance that this alias references
		__declspec(property(get=getNode)) NodePtr Node;
		virtual NodePtr getNode(void) = 0;
	};

	// Node
	//
	// todo: document when done
	struct __declspec(novtable) Node
	{
		// ResolvePath
		//
		// Resolves a relative path from this node to an Alias instance
		virtual AliasPtr ResolvePath(const tchar_t* path) = 0;

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

	// View
	//
	// todo: document when done
	struct __declspec(novtable) View
	{
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