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

#ifndef __VFSCONTAINERNODE_H_
#define __VFSCONTAINERNODE_H_
#pragma once

#include "VfsNode.h"				// Include VfsNode class declarations
#include <vector>					// Include STL vector<> declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VfsContainerNode
//
// Specialization of VfsNode for container objects (directories, etc)

class __declspec(novtable) VfsContainerNode : public VfsNode
{
public:

	// Destructor
	//
	virtual ~VfsContainerNode();

	//-------------------------------------------------------------------------
	// Member Functions

	VfsNode* AddChild(VfsNode* child) { m_children.push_back(child); return child; }

	VfsNode* FindChild(const char_t* path) { (path); return nullptr; }

	//-------------------------------------------------------------------------
	// Properties

protected:

	// Instance Constructor
	//
	VfsContainerNode(VfsNodeType type, VfsContainerNode* parent) : VfsNode(type, parent) {}

private:

	VfsContainerNode(const VfsContainerNode&);
	VfsContainerNode& operator=(const VfsContainerNode&);

	//-------------------------------------------------------------------------
	// Member Variables

	std::vector<VfsNode*>		m_children;			// Child nodes
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VFSNODE_H_
