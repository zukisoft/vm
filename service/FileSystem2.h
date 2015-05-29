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

#ifndef __FILESYSTEM2_H_
#define __FILESYSTEM2_H_
#pragma once

#include <memory>
#include <linux/statfs.h>

#pragma warning(push, 4)

// Forward Declarations
//
struct __declspec(novtable) Node;

//-----------------------------------------------------------------------------
// FileSystem
//
// Interface that must be implemented by a file system object

struct __declspec(novtable) FileSystem2
{
	//
	// MUST HAVE STATIC MOUNT() FUNCTION
	//

	// Stat
	//
	// Provides statistical information about the file system
	virtual void Stat(uapi::statfs* stats) = 0;

	// Root
	//
	// Gets a reference to the root file system node
	__declspec(property(get=getRoot)) std::shared_ptr<Node> Root;
	virtual std::shared_ptr<Node> getRoot(void) = 0;

	// Source
	//
	// Gets the device/name used as the source of the file system
	__declspec(property(get=getSource)) const char_t* Source;
	virtual const char_t* getSource(void) = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM2_H_