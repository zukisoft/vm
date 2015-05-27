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

#ifndef __NODETYPE_H_
#define __NODETYPE_H_
#pragma once

#include <linux/stat.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// NodeType
//
//
// Strogly typed enumeration for the S_IFxxx inode type constants, every node
// instance must indicate what type it is using one of these values

enum class NodeType
{
	BlockDevice			= LINUX_S_IFBLK,
	CharacterDevice		= LINUX_S_IFCHR,
	Directory			= LINUX_S_IFDIR,
	File				= LINUX_S_IFREG,
	Pipe				= LINUX_S_IFIFO,
	Socket				= LINUX_S_IFSOCK,
	SymbolicLink		= LINUX_S_IFLNK,
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NODETYPE_H_