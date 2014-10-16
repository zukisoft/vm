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

#ifndef __PATHSPLITTER_H_
#define __PATHSPLITTER_H_
#pragma once

#include <vector>
#include <linux/types.h>

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// PathSplitter
//
// Breaks up a POSIX path into branch and leaf components

class PathSplitter
{
public:

	// Constructor / Destructor
	//
	PathSplitter(const uapi::char_t* path);
	~PathSplitter()=default;

	// operator bool
	//
	// Indicates if either the leaf or branch is set to something
	operator bool() const { return (*m_leaf || *m_branch); }

	//-------------------------------------------------------------------------
	// Properties

	// Absolute
	//
	// Returns a flag if the original path was absolute (rooted)
	__declspec(property(get=getAbsolute)) bool Absolute;
	bool getAbsolute(void) const { return m_absolute; }

	// Branch
	//
	// Returns a pointer to the branch of the path
	__declspec(property(get=getBranch)) const uapi::char_t* Branch;
	const uapi::char_t* getBranch(void) const { return m_branch; }

	// Leaf
	//
	// Returns a pointer to the leaf of the path
	__declspec(property(get=getLeaf)) const uapi::char_t* Leaf;
	const uapi::char_t* getLeaf(void) const { return m_leaf; }

	// Relative
	//
	// Returns a flag if the original path was relative
	__declspec(property(get=getRelative)) bool Relative;
	bool getRelative(void) const { return !m_absolute; }

private:

	PathSplitter(const PathSplitter&)=delete;
	PathSplitter& operator=(const PathSplitter&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	std::vector<uapi::char_t>	m_path;				// Path string vector
	bool						m_absolute;			// Flag if path was absolute
	uapi::char_t*				m_branch;			// Pointer to the branch
	uapi::char_t*				m_leaf;				// Pointer to the leaf
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PATHSPLITTER_H_
