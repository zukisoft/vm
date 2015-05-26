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
#include "NewRootFileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem::Mount (static)
//
// Creates an instance of the file system
//
// Arguments:
//
//	source		- Source device path (ignored)
//	flags		- Standard mount options bitmask
//	data		- Extended mount options data
//	datalen		- Length, in bytes, of the extended mount options data

std::shared_ptr<FileSystem::Mount> NewRootFileSystem::Mount(const char_t* source, uint32_t flags, const void* data, size_t datalen)
{
	if(source == nullptr) throw LinuxException(LINUX_EFAULT);

	// Convert the flags and extended data into a MountOptions instance
	auto options = MountOptions::Create(flags, data, datalen);

	// Construct and return an instance of the private Mount class
	return std::make_shared<class Mount>(source, std::move(options));
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::Duplicate
//
// Duplicates this mount instance
//
// Arguments:
//
//	NONE

std::shared_ptr<FileSystem::Mount> NewRootFileSystem::Mount::Duplicate(void) const
{
	return nullptr;
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::getOptions
//
// Gets a pointer to the contained MountOptions instance

const MountOptions* NewRootFileSystem::Mount::getOptions(void) const
{
	return m_options.get();
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::Remount
//
// Remounts this mount point with different flags and arguments
//
// Arguments:
//
//	flags		- Standard mount options bitmask
//	data		- Extended mount options data
//	datalen		- Length, in bytes, of the extended mount options data

void NewRootFileSystem::Mount::Remount(uint32_t flags, const void* data, size_t datalen)
{
	// Allowed: MS_RDONLY, MS_SYNCHRONOUS, MS_MANDLOCK
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::getRoot
//
// Gets a pointer to the root node for this mount point

std::shared_ptr<FileSystem::Node> NewRootFileSystem::Mount::getRoot(void) const
{
	return nullptr;
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::getSource
//
// Retrieves the source device name for the mount point

const char_t* NewRootFileSystem::Mount::getSource(void) const
{
	return m_source.c_str();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
