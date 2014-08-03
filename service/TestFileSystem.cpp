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

#include "stdafx.h"
#include "TestFileSystem.h"

#pragma warning(push, 4)

// Constructor
TestFileSystem::TestFileSystem(const tchar_t* rootpath)
{
	m_root = CreateDirectoryNode(rootpath);
}

// Destructor
TestFileSystem::~TestFileSystem()
{
	m_root.reset();
}

// AllocateNodeIndex
int32_t TestFileSystem::AllocateNodeIndex(void)
{
	int32_t index;					// Allocated index value

	// Try to reuse a spent node index first, otherwise grab a new one.
	// If the returned value overflowed, there are no more indexes left
	if(!m_spentindexes.try_pop(index)) index = m_nextindex++;
	if(index < 0) throw LinuxException(LINUX_EDQUOT);

	return index;
}

// Mount (static)
std::unique_ptr<FileSystem> TestFileSystem::Mount(const tchar_t* device)
{
	// root = alias that becomes the root of this file system, must already exist
	return std::make_unique<TestFileSystem>(device);
}

// ReleaseNodeIndex
void TestFileSystem::RelaseNodeIndex(int32_t index)
{
	// The node indexes are reused aggressively for this file system,
	// push it into the spent index queue so that it will be grabbed
	// by AllocateNodeIndex() before a new index is generated
	m_spentindexes.push(index);
}

//
// TestFileSystem::Node
//

TestFileSystem::Node::~Node()
{
	OutputDebugString(L"~Node\r\n");

	// Close the underlying operating system handle
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);

	// Relase the index that was allocated for this node instance
	m_fs.RelaseNodeIndex(m_index);
}

//
// TestFileSystem::DirectoryNode
//

TestFileSystem::DirectoryNode::DirectoryNode(TestFileSystem& fs, HANDLE handle, int32_t index) : Node(fs, handle, index)
{
	FILE_BASIC_INFO basicinfo;				// Basic information about the node

	// Get basic information about the object handle refers to to check the attributes
	if(!GetFileInformationByHandleEx(handle, FileBasicInfo, &basicinfo, sizeof(FILE_BASIC_INFO))) throw LinuxException(LINUX_ENOENT, Win32Exception());

	// DirectoryNode must be constructed against a directory object handle
	if((basicinfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) throw LinuxException(LINUX_ENOTDIR);
}

std::shared_ptr<TestFileSystem::DirectoryNode> TestFileSystem::CreateDirectoryNode(const tchar_t* path)
{
	FILE_BASIC_INFO						info;			// Basic file information
	std::shared_ptr<DirectoryNode>		node;			// Root Node instance

	// NULL or zero-length path names are not supported, has to be set to something
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// Attempt to open the specified path with query-only access to pass into the Node instance
	HANDLE handle = CreateFile(std::to_tstring(path).c_str(), 0, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT, Win32Exception());

	try {

		// Query the basic information about the object to determine if it's a directory or not
		if(!GetFileInformationByHandleEx(handle, FileBasicInfo, &info, sizeof(FILE_BASIC_INFO))) throw LinuxException(LINUX_EACCES, Win32Exception());

		// Ensure that this object represents a directory
		if((info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)  throw LinuxException(LINUX_ENOTDIR);

		// Allocate an index for this node and construct the DirectoryNode instance
		int32_t index = AllocateNodeIndex();
		try { return std::make_shared<DirectoryNode>(*this, handle, index); }
		catch(...) { RelaseNodeIndex(index); throw; }
	}

	catch(...) { CloseHandle(handle); throw; }
}


//
// TestFileSystem::FileNode
//

TestFileSystem::FileNode::FileNode(TestFileSystem& fs, HANDLE handle, int32_t index) : Node(fs, handle, index)
{
	FILE_BASIC_INFO basicinfo;				// Basic information about the node

	// Get basic information about the object handle refers to to check the attributes
	if(!GetFileInformationByHandleEx(handle, FileBasicInfo, &basicinfo, sizeof(FILE_BASIC_INFO))) throw LinuxException(LINUX_ENOENT, Win32Exception());

	// FileNode cannot be constructed against a directory object handle
	if((basicinfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) throw LinuxException(LINUX_EISDIR);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
