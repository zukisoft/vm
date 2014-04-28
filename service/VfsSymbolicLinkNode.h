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

#ifndef __VFSSYMBOLICLINKNODE_H_
#define __VFSSYMBOLICLINKNODE_H_
#pragma once

#include <memory>
#include <string>
#include <linux/stat.h>
#include "Exception.h"
#include "StreamReader.h"
#include "VfsNode.h"

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VfsSymbolicLinkNodePtr
//
// Typedef of std::shared_ptr<VfsSymbolicLinkNode>

class VfsSymbolicLinkNode;
typedef std::shared_ptr<VfsSymbolicLinkNode> VfsSymbolicLinkNodePtr;

//-----------------------------------------------------------------------------
// VfsSymbolicLinkNode
//
// Virtual File System symbolic link node

class VfsSymbolicLinkNode : public VfsNode
{
public:

	// Instance Constructors
	//
	VfsSymbolicLinkNode(mode_t mode, const char_t* target) : VfsSymbolicLinkNode(mode, 0, 0, target) {}
	VfsSymbolicLinkNode(mode_t mode, uid_t uid, gid_t gid, const char_t* target);
	VfsSymbolicLinkNode(mode_t mode, StreamReader& data) : VfsSymbolicLinkNode(mode, 0, 0, data) {}
	VfsSymbolicLinkNode(mode_t mode, uid_t uid, gid_t gid, StreamReader& data) ;

	// Destructor
	//
	virtual ~VfsSymbolicLinkNode()=default;

	//-------------------------------------------------------------------------
	// Properties

	// Target
	//
	// Gets the target string for the symbolc link
	__declspec(property(get=getTarget)) const char_t* Target;
	const char_t* getTarget(void) const { return m_target.c_str(); }

private:

	VfsSymbolicLinkNode(const VfsSymbolicLinkNode&)=delete;
	VfsSymbolicLinkNode& operator=(const VfsSymbolicLinkNode&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	std::string				m_target;			// Symbolic link target
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VFSSYMBOLICLINKNODE_H_
