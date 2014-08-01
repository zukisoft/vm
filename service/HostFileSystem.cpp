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

//-----------------------------------------------------------------------------
// HostFileSystem Constructor
//
// Arguments:
//
//	devicename		- Device name passed into Mount(), must be a host directory

HostFileSystem::HostFileSystem(const char_t* devicename)
{
	FILE_BASIC_INFO				info;				// Basic file information

	//
	// TODO: Mounting options
	//

	// NULL or zero-length device names are not supported, has to be set to something
	if((devicename == nullptr) || (*devicename == 0)) throw LinuxException(LINUX_ENOENT);

	// Attempt to open the specified path with query-only access to pass into the Node instance
	HANDLE handle = CreateFile(std::to_tstring(devicename).c_str(), 0, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT, Win32Exception());

	try {

		// Query the basic information about the object to determine if it's a directory or not
		if(!GetFileInformationByHandleEx(handle, FileBasicInfo, &info, sizeof(FILE_BASIC_INFO))) throw LinuxException(LINUX_EACCES, Win32Exception());

		// If this is not a directory, it's not a valid mount point
		if((info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)  throw LinuxException(LINUX_ENOTDIR);

		// Create the root Node instance and hold a strong reference to it
		m_rootnode = std::make_shared<Node>(*this, handle, AllocateNodeIndex());
		
		// Create the root directory entry, which has no name, and hold a strong reference to that as well
		// todo: need to tell it that it's a mount point?
		m_rootalias = std::make_shared<DirectoryEntry>("", m_rootnode);
	}

	catch(const std::exception& ex) { CloseHandle(handle); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem Destructor

HostFileSystem::~HostFileSystem()
{
	// The file system objects must be destroyed before this object dies since
	// they may be maintaining references that will have serious problems
	// todo: kill everything here
	m_rootalias.reset();
	m_rootnode.reset();
}

//-----------------------------------------------------------------------------
// HostFileSystem::AllocateNodeIndex (private)
//
// Allocates a node index from the pool of available indexes
//
// Arguments:
//
//	NONE

int32_t HostFileSystem::AllocateNodeIndex(void)
{
	int32_t index;					// Allocated index value

	// Try to reuse a spent node index first, otherwise grab a new one.
	// If the returned value overflowed, there are no more indexes left
	if(!m_spentinodes.try_pop(index)) index = m_nextinode++;
	if(index < 0) throw LinuxException(LINUX_EDQUOT);

	return index;
}

//-----------------------------------------------------------------------------
// HostFileSystem::ReleaseNodeIndex (private)
//
// Releases a node index back into the pool of available indexes
//
// Arguments:
//
//	index		- Node index to be released

void HostFileSystem::RelaseNodeIndex(int32_t index)
{
	// The node indexes are reused aggressively for this file system,
	// push it into the spent index queue so that it will be grabbed
	// by AllocateNodeIndex() before a new index is generated
	m_spentinodes.push(index);
}

std::unique_ptr<FileSystem> HostFileSystem::Mount(int flags, const char_t* devicename, void* data)
{
	UNREFERENCED_PARAMETER(flags);
	UNREFERENCED_PARAMETER(data);

	// TODO: mounting options

	return std::make_unique<HostFileSystem>(devicename);
}

HostFileSystem::DirectoryEntry::DirectoryEntry(const char_t* name, const std::shared_ptr<HostFileSystem::Node>& node) : 
	FileSystem::DirectoryEntry(node), m_name(name)
{
	// TODO: check for NULL unless root node, perhaps make a special RootDirectoryEntry object instead
}



HostFileSystem::File::File(const std::shared_ptr<DirectoryEntry>& dentry, const std::shared_ptr<Node>& node) : FileSystem::File(dentry, node)
{
	// Both dentry and node must be valid objects
	_ASSERTE(dentry && node);
	if(!dentry || !node) throw Exception(E_UNEXPECTED);

	// If this is a directory, it can't be opened, the operations to create stuff should 
	// ultimately land in DirectoryEntry, this File class is just for reading and writing
	// not for creating/accessing children

	// TODO: correct mode, access, sharing, etc.
	// TODO: impersonation and whatnot too
	//m_handle = CreateFile(dentry->Path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	///if(m_handle == INVALID_HANDLE_VALUE) throw std::exception("TODO: new exception object");

	// file is now open
}

//-----------------------------------------------------------------------------
// HostFileSystem::File Destructor

HostFileSystem::File::~File()
{
	// Close the underlying operating system handle
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::File::Read
//
// Synchronously reads data from the underlying file
//
// Arguments:
//
//	buffer		- Output buffer
//	count		- Size of the output buffer, in bytes
//	pos			- Position within the file to start reading

uapi::size_t HostFileSystem::File::Read(void* buffer, uapi::size_t count, uapi::loff_t pos)
{
	DWORD read = 0;					// Number of bytes read from the file

	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
	std::lock_guard<std::recursive_mutex> critsec(m_lock);

#ifdef _M_X64
	// Count cannot exceed UINT32_MAX; ReadFile() accepts a 32-bit argument
	if(count > UINT32_MAX) throw LinuxException(LINUX_EFBIG);
#endif

	// A null buffer pointer and non-zero length is not going to work
	if((buffer == nullptr) && (count > 0)) throw LinuxException(LINUX_EFAULT);

	// If the position requested does not match the current file pointer, attempt to change it first
	if((pos != m_position) && (pos != Seek(pos, LINUX_SEEK_SET))) throw LinuxException(LINUX_EINVAL);
	if(!ReadFile(m_handle, buffer, count, &read, nullptr)) throw LinuxException(LINUX_EINVAL, Win32Exception());

	m_position += read;					// Advance the cached file pointer
	return read;						// Return number of bytes read

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
	std::lock_guard<std::recursive_mutex> critsec(m_lock);

	// These constants are the same and can be used interchangably as long as that's the case
	static_assert(LINUX_SEEK_SET == FILE_BEGIN, "LINUX_SEEK_END constant is not the same as FILE_BEGIN");
	static_assert(LINUX_SEEK_CUR == FILE_CURRENT, "LINUX_SEEK_END constant is not the same as FILE_CURRENT");
	static_assert(LINUX_SEEK_END == FILE_END, "LINUX_SEEK_END constant is not the same as FILE_END");

	// Validate the origin argument, this comes from the hosted process
	if((origin < FILE_BEGIN) || (origin > FILE_END)) throw LinuxException(LINUX_EINVAL);

	// SetFilePointerEx() expects LARGE_INTEGERs rather than loff_t values
	LARGE_INTEGER distance, position;
	distance.QuadPart = offset;

	// Attempt to change the file position
	if(!SetFilePointerEx(m_handle, distance, &position, origin)) throw LinuxException(LINUX_EINVAL, Win32Exception());

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
	std::lock_guard<std::recursive_mutex> critsec(m_lock);

	// TODO: needs to have/check for write access to the file --> EINVAL if not

	if(!FlushFileBuffers(m_handle)) throw LinuxException(LINUX_EIO, Win32Exception());
}

//-----------------------------------------------------------------------------
// HostFileSystem::Write
//
// Synchronously writes data to the underyling file
//
// Arguments:
//
//	buffer		- Source data input buffer
//	count		- Size of the input buffer, in bytes
//	pos			- Position within the file to begin writing

uapi::size_t HostFileSystem::File::Write(void* buffer, uapi::size_t count, uapi::loff_t pos)
{
	DWORD written = 0;					// Number of bytes written to the file

	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
	std::lock_guard<std::recursive_mutex> critsec(m_lock);

#ifdef _M_X64
	// Count cannot exceed UINT32_MAX; WriteFile() accepts a 32-bit argument
	if(count > UINT32_MAX) throw LinuxException(LINUX_EFBIG);
#endif

	// A null buffer pointer and non-zero length is not going to work
	if((buffer == nullptr) && (count > 0)) throw LinuxException(LINUX_EFAULT);

	// If the position requested does not match the current file pointer, attempt to change it first
	if((pos != m_position) && (pos != Seek(pos, LINUX_SEEK_SET))) throw LinuxException(LINUX_EINVAL);
	if(!WriteFile(m_handle, buffer, count, &written, nullptr)) throw LinuxException(LINUX_EINVAL, Win32Exception());

	m_position += written;				// Advance the cached file pointer
	return written;						// Return number of bytes written
}


//HostFileSystem::Node::Node(HANDLE handle) : m_handle(handle)
//{
//}

//-----------------------------------------------------------------------------
// HostFileSystem::Node Destructor

HostFileSystem::Node::~Node()
{
	// Close the underlying operating system object handle
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);

	// Release the node index from the parent file system instance
	m_fs.RelaseNodeIndex(static_cast<int32_t>(m_index));
}


std::shared_ptr<FileSystem::File> HostFileSystem::Node::OpenFile(const std::shared_ptr<FileSystem::DirectoryEntry>& dentry)
{
	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);

	return nullptr;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
