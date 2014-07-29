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
#include "HostFileSystem.h"

#pragma warning(push, 4)

std::unique_ptr<FileSystem> HostFileSystem::Mount(int flags, const char_t* devicename, void* data)
{
	UNREFERENCED_PARAMETER(flags);
	UNREFERENCED_PARAMETER(devicename);
	UNREFERENCED_PARAMETER(data);

	return nullptr;
}

HostFileSystem::DirectoryEntry::DirectoryEntry(const char_t* name, const tchar_t* path) : m_name(name), m_path(path)
{
	// TODO: check for NULL unless root node, perhaps make a special RootDirectoryEntry object instead
}

HostFileSystem::File::File(const std::shared_ptr<DirectoryEntry>& dentry, const std::shared_ptr<Node>& node) : FileSystem::File(dentry, node)
{
	// Both dentry and node must be valid objects
	_ASSERTE(dentry && node);
	if(!dentry || !node) throw std::exception("TODO: new exception object");

	// If this is a directory, it can't be opened, the operations to create stuff should 
	// ultimately land in DirectoryEntry, this File class is just for reading and writing
	// not for creating/accessing children

	// TODO: correct mode, access, sharing, etc.
	// TODO: impersonation and whatnot too
	m_handle = CreateFile(dentry->Path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(m_handle == INVALID_HANDLE_VALUE) throw std::exception("TODO: new exception object");

	// file is now open
}

HostFileSystem::File::~File()
{
	// Close the underlying operating system handle
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

uapi::size_t HostFileSystem::File::Read(void* buffer, uapi::size_t count, uapi::loff_t pos)
{
	DWORD read = 0;

	// count cannot be > UINT32_MAX in 64 bit builds --> throw an error

	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
	_ASSERTE(buffer);

	// If the position requested does not match the current file pointer, attempt to change it
	if((pos != m_position) && (pos != Seek(pos, LINUX_SEEK_SET))) throw std::exception("TODO: new exception object");

	// TODO: need to think about if these should throw or return error codes, that's why I need
	// the new exception object - it should have a Linux status code and a Windows one, use something
	// generic rather than trying to map them out if only one is specified
	// xxxException(LINUX_EINVAL, GetLastError());
	if(!ReadFile(m_handle, buffer, count, &read, nullptr)) throw std::exception("TODO: new exception object");

	return read;
}

//-----------------------------------------------------------------------------
// HostFileSystem::File::Seek
//
// Changes the file pointer position
//
// Arguments:
//
//	offset		- New offset, based on origin
//	origin		- Origin from whence to move the file pointer

uapi::loff_t HostFileSystem::File::Seek(uapi::loff_t offset, int origin)
{
	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);

	// These constants are the same and can be used interchangably as long as that's the case
	static_assert(LINUX_SEEK_SET == FILE_BEGIN, "LINUX_SEEK_END constant is not the same as FILE_BEGIN");
	static_assert(LINUX_SEEK_CUR == FILE_CURRENT, "LINUX_SEEK_END constant is not the same as FILE_CURRENT");
	static_assert(LINUX_SEEK_END == FILE_END, "LINUX_SEEK_END constant is not the same as FILE_END");

	// SetFilePointerEx() expects LARGE_INTEGERs rather than loff_t values
	LARGE_INTEGER distance, position;
	distance.QuadPart = offset;

	// Attempt to change the file position
	if(!SetFilePointerEx(m_handle, distance, &position, origin)) throw std::exception("TODO: new exception object");

	m_position = position.QuadPart;			// Store the new pointer position
	return m_position;						// Return the new position
}

//-----------------------------------------------------------------------------
// HostFileSystem::File::Sync
//
// Flushes any buffered data to the underlying storage medium
//
// Arguments:
//
//	NONE

void HostFileSystem::File::Sync(void)
{
	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
	
	// TODO: needs to have write access, should be a member variable
	if(!FlushFileBuffers(m_handle)) throw std::exception("TODO: new exception object");
}

uapi::size_t HostFileSystem::File::Write(void* buffer, uapi::size_t count, uapi::loff_t pos)
{
	DWORD written = 0;

	// count cannot be > UINT32_MAX in 64 bit builds --> throw an error

	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
	_ASSERTE(buffer);

	// If the position requested does not match the current file pointer, attempt to change it
	if((pos != m_position) && (pos != Seek(pos, LINUX_SEEK_SET))) throw std::exception("TODO: new exception object");

	// TODO: need to think about if these should throw or return error codes, that's why I need
	// the new exception object - it should have a Linux status code and a Windows one, use something
	// generic rather than trying to map them out if only one is specified
	// xxxException(LINUX_EINVAL, GetLastError());
	if(!WriteFile(m_handle, buffer, count, &written, nullptr)) throw std::exception("TODO: new exception object");

	return written;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
