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

#ifndef __VFSNODE_H_
#define __VFSNODE_H_
#pragma once

#include <string>					// Include STL string<> declarations

// VfsContainerNode (Forward Declaration)
//
class VfsContainerNode;

#pragma warning(push, 4)			// Enable maximum compiler warnings

// move me
enum class VfsNodeType {

	Directory		= 0,
	File,
};

//-----------------------------------------------------------------------------
// VfsNode
//
// Base class for all virtual file system nodes
//
// TODO: Move name to a string table?
// TODO: Can these be allocated on a private heap 
// TODO: Need private object security -- map uid/gid/mask to a collection
// TODO: Using a vector<> for children is going to be O(n) --> terrible

class __declspec(novtable) VfsNode
{
public:

	// Destructor
	//
	virtual ~VfsNode() {}

	//-------------------------------------------------------------------------
	// Member Functions

	//-------------------------------------------------------------------------
	// Properties

	__declspec(property(get=getMode, put=putMode)) uint32_t Mode;
	uint32_t getMode(void) const { return m_mode; }
	void putMode(uint32_t value) { m_mode = value; }

	__declspec(property(get=getType)) VfsNodeType Type;
	VfsNodeType getType(void) const { return m_type; }

protected:

	// Instance Constructor
	//
	VfsNode(VfsNodeType type, VfsContainerNode* parent) : m_type(type), m_parent(parent) {}

private:

	VfsNode(const VfsNode&);
	VfsNode& operator=(const VfsNode&);

	//-------------------------------------------------------------------------
	// Member Variables

	VfsNodeType					m_type;				// Node type
	std::string					m_name;				// Name
	VfsContainerNode*			m_parent;			// Parent node
	
	// temporary?  Need a security descriptor / private object security
	// except for temporary files, so maybe that doesn't count here?

	uint32_t					m_mode;				// Node mode flags
	uint32_t					m_uid;				// Node owner
	uint32_t					m_gid;				// Node owner
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VFSNODE_H_
