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

#ifndef __VFSDIRECTORYNODE_H_
#define __VFSDIRECTORYNODE_H_
#pragma once
				
#include <map>
#include <memory>
#include <string>
#include <linux/stat.h>
#include <linux/types.h>
#include "AutoReaderLock.h"
#include "AutoWriterLock.h"
#include "ReaderWriterLock.h"
#include "VfsNode.h"

#pragma warning(push, 4)			

//-----------------------------------------------------------------------------
// VfsDirectoryNodePtr
//
// Typedef of std::shared_ptr<VfsDirectoryNode>

class VfsDirectoryNode;
typedef std::shared_ptr<VfsDirectoryNode> VfsDirectoryNodePtr;

//-----------------------------------------------------------------------------
// VfsDirectoryNode
//
// Virtual File System directory node

class VfsDirectoryNode : public VfsNode
{
public:

	// Instance Constructors
	//
	VfsDirectoryNode(const VfsDirectoryNodePtr& parent, uapi::mode_t mode) : VfsDirectoryNode(parent, mode, 0, 0) {}
	VfsDirectoryNode(const VfsDirectoryNodePtr& parent, uapi::mode_t mode, uapi::uid_t uid, uapi::gid_t gid);

	// Destructor
	//
	virtual ~VfsDirectoryNode();

	//-------------------------------------------------------------------------
	// Member Functions

	// AddAlias
	//
	// Adds an alias to the directory
	void AddAlias(const char_t* alias, const VfsNodePtr& node);

	// GetAlias
	//
	// Locates an alias within the directory
	VfsNodePtr GetAlias(const char_t* alias);

	// RemoveAlias
	//
	// Removes an existing alias from the directory
	void RemoveAlias(const char_t* alias);
	void RemoveAlias(const VfsNodePtr& alias)=delete;	// TODO
	void RemoveAlias(int32_t alias)=delete;				// TODO

	//-------------------------------------------------------------------------
	// Properties

	// Parent
	//
	// Returns a reference to this directory's parent directory
	__declspec(property(get=getParent)) const VfsDirectoryNodePtr Parent;
	const VfsDirectoryNodePtr getParent(void) const { return m_parent.lock(); }

private:

	VfsDirectoryNode(const VfsDirectoryNode&);
	VfsDirectoryNode& operator=(const VfsDirectoryNode&);

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// AliasCollection
	//
	// Defines the contained alias collection type
	typedef std::map<std::string, std::shared_ptr<VfsNode>>	AliasCollection;

	// AliasIterator
	//
	// Defines the iterator type for the contained alias collection
	typedef std::map<std::string, std::shared_ptr<VfsNode>>::iterator AliasIterator;

	//-------------------------------------------------------------------------
	// Member Variables

	std::weak_ptr<VfsDirectoryNode>	m_parent;		// Parent directory
	AliasCollection					m_aliases;		// Contained aliases
	static ReaderWriterLock			s_lock;			// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VFSDIRECTORYNODE_H_
