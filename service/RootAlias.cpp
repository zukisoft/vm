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

#include "stdafx.h"
#include "RootAlias.h"
#include "LinuxException.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootAlias Constructor
//
// Arguments:
//
//	node		- Node to attach to this alias

RootAlias::RootAlias(std::shared_ptr<FileSystem::Node> node) : m_node(std::move(node))
{
}

//-----------------------------------------------------------------------------
// RootAlias::GetName
//
// Reads the name assigned to this alias
//
// Arguments:
//
//	buffer			- Output buffer
//	count			- Size of the output buffer, in bytes

uapi::size_t RootAlias::GetName(char_t* buffer, size_t count) const
{
	UNREFERENCED_PARAMETER(count);

	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);

	return 0;
}

//-----------------------------------------------------------------------------
// RootAlias::getName
//
// Gets the name assigned to this alias

std::string RootAlias::getName(void) const
{
	return std::string();
}

//-----------------------------------------------------------------------------
// RootAlias::getNode
//
// Gets the node to which this alias refers

std::shared_ptr<FileSystem::Node> RootAlias::getNode(void) const
{
	return m_node;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
