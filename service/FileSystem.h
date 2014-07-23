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

#ifndef __FILESYSTEM_H_
#define __FILESYSTEM_H_
#pragma once

#include <atomic>
#include <concurrent_priority_queue.h>

#pragma warning(push, 4)			

//-----------------------------------------------------------------------------
// Class FileSystem
//
// Base class for all file system classes

class FileSystem
{
public:

	// Node
	//
	// words
	class Node
	{
	public:

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;
	};

	// Directory
	//
	// words
	class Directory
	{
	public:

	private:

		Directory(const Directory&)=delete;
		Directory& operator=(const Directory&)=delete;
	};

	FileSystem()=default;
	~FileSystem()=default;

	// BlockSize
	//
	// Gets the file system block size
	__declspec(property(get=getBlockSize)) size_t BlockSize;
	virtual size_t getBlockSize(void) const = 0;
	
	// Name
	//
	// Gets the file system name
	__declspec(property(get=getName)) const char_t* Name;
	virtual const char_t* getName(void) const = 0;

protected:

	// AllocateNodeIndex
	//
	// Allocates a node index for this filesystem instance
	uint32_t AllocateNodeIndex(void);

	// ReleaseNodeIndex
	//
	// Releases a node index for reuse in this filesystem
	void ReleaseNodeIndex(uint32_t index);

private:

	FileSystem(const FileSystem&)=delete;
	FileSystem& operator=(const FileSystem&)=delete;

	// m_nextindex
	//
	// Next sequential node index value
	std::atomic<uint32_t> m_nextindex;

	// m_spentindexes
	//
	// Priority queue used to recycle node indexes; lower values will be used first
	Concurrency::concurrent_priority_queue<uint32_t, std::greater<uint32_t>> m_spentindexes;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM_H_
