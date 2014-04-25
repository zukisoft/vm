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

#ifndef __VFSSEARCHRESULT_H_
#define __VFSSEARCHRESULT_H_
#pragma once

#include "VfsNode.h"
#include "VfsNodePtr.h"

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VfsSearch
//
// Poorly named enumeration indicating what node within a search path has
// been returned from VirtualFileSystem::Find

enum class VfsSearch
{
	Found			= 0,		// Path resolved to a specific node
	FoundParent,				// Path resolved to the immediate ancestor
	FoundAncestor,				// Path resolved to a non-immediate ancestor
};

//-----------------------------------------------------------------------------
// VfsSearchResult
//
// Result object returned from a VFS path search.  This result can indicate one of
// three conditions.  In all cases the node where the search terminated is provided
// so that the caller can decide if there is something useful to do with it.

class VfsSearchResult
{
public:

	// Constructor
	//
	VfsSearchResult(VfsSearch result, VfsNodePtr node) : m_result(result), m_node(node) {}

	//-------------------------------------------------------------------------
	// Member Functions

	//-------------------------------------------------------------------------
	// Properties

	// Result
	//
	// Exposes the result value for the search operation
	__declspec(property(get=getResult)) VfsSearch Result;
	VfsSearch getResult(void) const { return m_result; }

private:

	//-------------------------------------------------------------------------
	// Member Variables

	VfsSearch				m_result;			// Result of search
	VfsNodePtr				m_node;				// Node located by search
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VFSSEARCHRESULT_H_
