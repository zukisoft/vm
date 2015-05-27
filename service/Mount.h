//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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

#ifndef __MOUNT_H_
#define __MOUNT_H_
#pragma once

#include "FileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Mount
//
// todo: words

struct __declspec(novtable) Mount
{
	// Duplicate
	//
	// Duplicates this mount instance
	virtual std::shared_ptr<Mount> Duplicate(void) = 0;

	// Remount
	//
	// Remounts this mount point with different flags and arguments
	virtual void Remount(uint32_t flags, const void* data, size_t datalen) = 0;

	// Node
	//
	// Gets a reference to the root node of this mount
	__declspec(property(get=getNode)) std::shared_ptr<FileSystem::Node> Node;
	virtual std::shared_ptr<FileSystem::Node> getNode(void) = 0;

	// Source
	//
	// Retrieves the source device name used to create the mount
	__declspec(property(get=getSource)) const char_t* Source;
	virtual const char_t* getSource(void) = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MOUNT_H_