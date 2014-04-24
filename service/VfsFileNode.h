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

#ifndef __VFSFILENODE_H_
#define __VFSFILENODE_H_
#pragma once
				
#include <linux/stat.h>
#include <memory>
#include "tstring.h"
#include "Exception.h"
#include "StreamReader.h"
#include "VfsNode.h"
#include "Win32Exception.h"

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VfsFileNode
//
// Virtual File System regular file node

class VfsFileNode : public VfsNode
{
public:

	// Instance Constructors
	//
	VfsFileNode(mode_t mode) : VfsFileNode(mode, 0, 0) {}
	VfsFileNode(mode_t mode, uid_t uid, gid_t gid);
	VfsFileNode(mode_t mode, StreamReader& data) : VfsFileNode(mode, 0, 0, data) {}
	VfsFileNode(mode_t mode, uid_t uid, gid_t gid, StreamReader& data);

	// Destructor
	//
	virtual ~VfsFileNode();

	//-------------------------------------------------------------------------
	// Member Functions

	//-------------------------------------------------------------------------
	// Properties

private:

	VfsFileNode(const VfsFileNode&);
	VfsFileNode& operator=(const VfsFileNode&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	//-------------------------------------------------------------------------
	// Member Variables

	HANDLE			m_handle = INVALID_HANDLE_VALUE;		// Underlying HANDLE
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VFSFILENODE_H_
