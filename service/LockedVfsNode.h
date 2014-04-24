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

#ifndef __LOCKEDVFSNODE_H_
#define __LOCKEDVFSNODE_H_
#pragma once

#include "VfsNode.h"

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// LockedVfsNode
//
// Wrapper class for a VfsNode pointer that automaticaly calls .Release() on it
// when destroyed or falls out of scope

class LockedVfsNode
{
public:

	// Constructor
	//
	LockedVfsNode(VfsNode* const node) : m_node(node) {}

	// Destructor
	//
	~LockedVfsNode() { m_node->Release(); }

	//-------------------------------------------------------------------------
	// Overloaded Operators

	// Member Selection Operator
	//
	VfsNode* operator->() const { return m_node; }

private:

	LockedVfsNode(const LockedVfsNode&);
	LockedVfsNode& operator=(const LockedVfsNode&);

	//-------------------------------------------------------------------------
	// Member Variables

	VfsNode* const			m_node;				// Contained node pointer
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __LOCKEDVFSNODE_H_
