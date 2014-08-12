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

struct __declspec(novtable) FileSystem
{
	// AliasPtr
	//
	// Alias for an std::shared_ptr<Alias>
	struct Alias;
	using AliasPtr = std::shared_ptr<Alias>;

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

	// AliasState
	//
	// Defines the state of an alias
	enum class AliasState
	{
		Attached		= 0,		// Alias is attached to a Node
		Detached		= 1,		// Alias is not attached to a Node
	};

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

	// Alias
	//
	// todo: document when done
	struct __declspec(novtable) Alias
	{
		// Mount
		//
		// Mounts a node instance to this alias instance
		virtual void Mount(const NodePtr& node) = 0;

		// Unmount
		//
		// Removes a mounted node from this alias instance
		virtual void Unmount(void) = 0;

		// MountPoint
		//
		// Determines if this alias is acting as a mount point
		__declspec(property(get=getMountPoint)) bool MountPoint;
		virtual bool getMountPoint(void) = 0;

		// Name
		//
		// Gets the name assigned to this alias instance
		__declspec(property(get=getName)) const tchar_t* Name;
		virtual const tchar_t* getName(void) = 0;

		// Node
		//
		// Gets a pointer to the underlying node for this alias
		//__declspec(property(get=getNode)) std::shared_ptr<Node> Node;  todo - name clash
		virtual NodePtr getNode(void) = 0;

		// State
		//
		// Gets the state (attached/detached) of this alias instance
		__declspec(property(get=getState)) AliasState State;
		virtual AliasState getState(void) = 0;
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
		// Creates a directory node as a child of this node instance
		//virtual NodePtr CreateDirectory(const tchar_t* name, uapi::mode_t mode) = 0;

		// CreateSymbolicLink
		//
		// Creates a symbolic link node as a child of this node instance
		//virtual NodePtr CreateSymbolicLink(const tchar_t* name, const tchar_t* target) = 0;

		// Index
		//
		// Gets the node index
		__declspec(property(get=getIndex)) uint32_t Index;
		virtual uint32_t getIndex(void) = 0;

		// MountPoint
		//
		// Indicates if this node represents a mount point
		// todo: don't need this
		//__declspec(property(get=getMountPoint)) bool MountPoint;
		//virtual bool getMountPoint(void) = 0;

		// Type
		//
		// Gets the node type
		__declspec(property(get=getType)) NodeType Type;
		virtual NodeType getType(void) = 0;
	};

	//
	// FileSystem Members
	//
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM_H_