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

#ifndef __ALIAS_H_
#define __ALIAS_H_
#pragma once

#include "Namespace.h"

#pragma warning(push, 4)

// Forward Declarations
//
struct __declspec(novtable) Node;

//-----------------------------------------------------------------------------
// Alias
//
// Interface that must be implemented by a file system alias.  Similiar to a 
// linux dentry, an alias defines a name associated with a file system node.
//
// Alias instances may support mounting, in which a reference to a foreign
// node can be provided to mask/override how the alias will be resolved.  If an
// alias does not support mounting, the Mount and Unmount functions should throw
// LinuxException(LINUX_EPERM, Exception(E_NOTIMPL))

struct __declspec(novtable) Alias
{
	// Follow
	//
	// Follows this alias to the file system node that it refers to
	virtual std::shared_ptr<Node> Follow(const std::shared_ptr<Namespace>& ns) = 0;

	// Mount
	//
	// Adds a mountpoint node to this alias, obscuring any existing node in the same namespace
	virtual void Mount(const std::shared_ptr<Namespace>& ns, const std::shared_ptr<Node>& node) = 0;

	// Unmount
	//
	// Removes a mountpoint node from this alias
	virtual void Unmount(const std::shared_ptr<Namespace>& ns, const std::shared_ptr<Node>& node) = 0;

	// Name
	//
	// Gets the name associated with the alias
	__declspec(property(get=getName)) const char_t* Name;
	virtual const char_t* getName(void) = 0;

	// Parent
	//
	// Gets the parent alias of this alias instance, or nullptr if none exists
	__declspec(property(get=getParent)) std::shared_ptr<Alias> Parent;
	virtual std::shared_ptr<Alias> getParent(void) = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ALIAS_H_