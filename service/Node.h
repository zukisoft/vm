//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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

#ifndef __NODE_H_
#define __NODE_H_
#pragma once

#include <memory>
#include <linux/stat.h>
#include "NodeType.h"

#pragma warning(push, 4)

// Forward Declarations
//
struct __declspec(novtable) Alias;
struct __declspec(novtable) FileSystem2;
struct __declspec(novtable) Handle;

//-----------------------------------------------------------------------------
// Node
//
// Interface that must be implemented for a file system node object.  A node
// represents any object (file, directory, socket, etc) that is part of a file
// system.
//
// Nodes are unnamed objects, internally they should be referenced by an index
// that is unique within the file system; the Alias interface (see Alias.h)
// provides the means to map a name to a specific node.  A single node can be
// referenced by multiple aliases, for example in the case of a hard link.
//
// Nodes are responsible for their own path resolution, this is done to allow
// for optimizations or shortcuts specific to a file system.  For example, a
// virtualized file system that sits atop a physical one on the host system
// can ignore traversing each of its child nodes and merely send the path to
// the host OS implementation instead, and construct the necessary Alias/Node
// instances around the returned object(s).

struct __declspec(novtable) Node
{
	// DemandPermission
	//
	// Demands read/write/execute permissions for the node (MAY_READ, MAY_WRITE, MAY_EXECUTE)
	virtual void DemandPermission(uapi::mode_t mode) = 0;

	// Open
	//
	// Creates a Handle instance against this node
	virtual std::shared_ptr<Handle> Open(const std::shared_ptr<Alias>& alias, int flags) = 0;

	// Lookup
	//
	// Resolves a relative path to an alias from this node
	virtual std::shared_ptr<Alias> Lookup(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& current, 
		const char_t* path, int flags, int* symlinks) = 0;
		
	// Stat
	//
	// Provides statistical information about the node
	virtual void Stat(uapi::stat* stats) = 0;

	// FileSystem
	//
	// Gets a reference to this node's parent file system instance
	__declspec(property(get=getFileSystem)) std::shared_ptr<FileSystem2> FileSystem;
	virtual std::shared_ptr<FileSystem2> getFileSystem(void) = 0;

	// Type
	//
	// Gets the type of node being represented in the derived object instance
	__declspec(property(get=getType)) NodeType Type;
	virtual NodeType getType(void) = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NODE_H_