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

#ifndef __VFSNODEPTR_H_
#define __VFSNODEPTR_H_
#pragma once

#include "VfsNode.h"

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VfsNodePtr
//
// Wrapper class for a VfsNode pointer that automaticaly calls .Release() on it
// when destroyed or falls out of scope
//
// Not sure I want to keep it this way, but construct/assignment against a raw
// VfsNode pointer will not invoke AddRef(), whereas construct/assignment from
// another VfsNodePtr instance will:
//
//	VfsNodePtr(VfsNode*)		--> No AddRef()
//	VfsNodePtr(VfsNodePtr&)		--> AddRef()
//	operator=(VfsNode*)			--> No AddRef()
//	operator=(VfsNodePtr&)		--> AddRef()

class VfsNodePtr
{
public:

	// Constructors
	//
	VfsNodePtr(VfsNode* node) : m_node(node) {}
	VfsNodePtr(const VfsNodePtr& rhs) { m_node = (rhs.m_node) ? rhs.m_node->AddRef() : nullptr; }

	// Destructor
	//
	~VfsNodePtr() { if(m_node) m_node->Release(); }

	//-------------------------------------------------------------------------
	// Overloaded Operators

	// Assignment operator (raw VfsNode pointer)
	//
	VfsNodePtr& operator=(VfsNode* node) 
	{ 
		if(m_node) m_node->Release(); 
		m_node = node; 
		return *this; 
	}

	// Assignment operator (VfsNodePtr instance)
	//
	VfsNodePtr& operator=(const VfsNodePtr& rhs) 
	{
		if(m_node) m_node->Release();
		m_node = (rhs.m_node) ? rhs.m_node->AddRef() : nullptr;
		return *this;
	}

	// Member Selection operator
	//
	VfsNode* const operator->() const { return m_node; }

	//-------------------------------------------------------------------------
	// Member Functions

	// Detach
	//
	// Detaches the VfsNode pointer from this class instance
	VfsNode* Detach(void) 
	{ 
		VfsNode* result = m_node; 
		m_node = nullptr; 
		return result; 
	}

private:

	//-------------------------------------------------------------------------
	// Member Variables

	VfsNode* 				m_node;				// Contained node pointer
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VFSNODEPTR_H_
