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

#ifndef __VFSRESOLVERESULT_H_
#define __VFSRESOLVERESULT_H_
#pragma once

#include "VfsDirectoryNode.h"
#include "VfsNode.h"

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VfsResolveStatus
//
// Enumeration indicating the status of a path resolution operation

enum class VfsResolveStatus : int32_t
{
	FoundLeaf				= 0,						// Leaf node was found
	FoundBranch				= 1,						// Branch node was found
	AccessDenied			= -LINUX_EACCES,			// Access denied traversing
	BranchNotFound			= -LINUX_ENOENT,			// Branch node not found
	BranchNotDirectory		= -LINUX_ENOTDIR,			// Branch node not directory
	BranchRecursionLimit	= -LINUX_ELOOP,				// Branch recursion limit
	PathTooLong				= -LINUX_ENAMETOOLONG,		// Resolved path too long
};

//-----------------------------------------------------------------------------
// VfsResolveResult
//
// Result object returned from a VFS path search

class VfsResolveResult
{
public:

	// Instance Constructors
	//
	VfsResolveResult(VfsResolveStatus status) 
		: m_status(status), m_branch(nullptr), m_leaf(nullptr) {}
	VfsResolveResult(VfsResolveStatus status, const VfsDirectoryNodePtr& branch, const std::string& alias) 
		: m_status(status), m_branch(branch), m_leaf(nullptr), m_alias(alias) {}
	VfsResolveResult(VfsResolveStatus status, const VfsDirectoryNodePtr& branch, const VfsNodePtr& leaf, const std::string& alias) 
		: m_status(status), m_branch(branch), m_leaf(leaf), m_alias(alias) {}
	VfsResolveResult(const VfsResolveResult&)=default;

	//-------------------------------------------------------------------------
	// Overloaded Operators

	// Assignment operator
	//
	VfsResolveResult& operator=(const VfsResolveResult& rhs)
	{
		// Disallow self-assignment for this class
		if(&rhs == this) return *this;

		m_status = rhs.m_status;
		m_branch = rhs.m_branch;
		m_leaf = rhs.m_leaf;
		return *this;
	}

	// Logical NOT operator
	//
	bool operator!(void) { return (static_cast<int32_t>(m_status) < 0); }

	//-------------------------------------------------------------------------
	// Properties

	// Alias
	//
	// Gets the alias name for the leaf node as viewed from the path
	__declspec(property(get=getAlias)) const char_t* Alias;
	const char_t* getAlias(void) const { return m_alias.c_str(); }

	// Branch
	//
	// Gets the branch node returned from the resolve operation; or nullptr
	__declspec(property(get=getBranch)) const VfsDirectoryNodePtr& Branch;
	const VfsDirectoryNodePtr& getBranch(void) const { return m_branch; }

	// Leaf
	//
	// Gets the leaf node returned from the resolve operation, or nullptr
	__declspec(property(get=getLeaf)) const VfsNodePtr& Leaf;
	const VfsNodePtr& getLeaf(void) const { return m_leaf; }

	// Status
	//
	// Exposes the result status from the path resolution operation
	__declspec(property(get=getStatus)) VfsResolveStatus Status;
	VfsResolveStatus getStatus(void) const { return m_status; }

private:

	//-------------------------------------------------------------------------
	// Member Variables

	VfsResolveStatus		m_status;			// Result of search
	VfsDirectoryNodePtr		m_branch;			// Branch node
	VfsNodePtr				m_leaf;				// Leaf node
	std::string				m_alias;			// Leaf node alias
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VFSRESOLVERESULT_H_
