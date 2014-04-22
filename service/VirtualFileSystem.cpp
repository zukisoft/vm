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
#include "VirtualFileSystem.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

#pragma push_macro("RPC_TSTR")
#undef RPC_TSTR
#ifdef _UNICODE
#define RPC_TSTR	RPC_WSTR
#else
#define RPC_TSTR	RPC_CSTR
#endif

//
// TODO: I DESPISE STD::STRING AND ALL THIS APPEND THIS APPEND THAT CRAP
// (but it works for now)
//

//-----------------------------------------------------------------------------
// VirtualFileSystem Constructor
//
// Arguments:
//
//	tempdir		- Temporary directory for virtual files, can be NULL

VirtualFileSystem::VirtualFileSystem(const tchar_t* tempdir)
{
	// Use the temporary directory specified or generate one as necessary
	m_tempdir = (tempdir) ? tempdir : GetTemporaryDirectory();

	// The temporary path needs to end with a trailing backslash
	if(m_tempdir.compare(m_tempdir.length() - 1, 1, L"\\") != 0) m_tempdir.append(L"\\");

	// Add the GUID to the provided temporary directory
	m_tempdir.append(GetUuid()) += L"\\";

	// Create the directory for temporary file storage
	if(!CreateDirectory(m_tempdir.c_str(), nullptr)) throw Win32Exception();

	// Construct the root file system node
	m_root = new VfsDirectoryNode(nullptr);
}

//-----------------------------------------------------------------------------
// VirtualFileSystem Destructor

VirtualFileSystem::~VirtualFileSystem()
{
	// Deletion of the root node will cascade delete the virtual file system
	delete m_root;

	// Attempt to remove the temporary file directory, but don't throw
	RemoveDirectory(m_tempdir.c_str());
}

//-----------------------------------------------------------------------------
// VirtualFileSystem::GetTemporaryDirectory (static, private)
//
// Generates a temporary directory name for the current process if one
// was not specified in the class constructor
//
// Arguents:
//
//	NONE

std::tstring VirtualFileSystem::GetTemporaryDirectory(void)
{
	tchar_t		temppath[MAX_PATH + 1];			// Path buffer

	// Get the temporary path for the current process identity
	if(GetTempPath(MAX_PATH + 1, temppath) == 0) throw Win32Exception();

	return std::tstring(temppath);
}

//-----------------------------------------------------------------------------
// VirtualFileSystem::GetUuid (static, private)
//
// Generates a UUID string
//
// Arguments:
//
//	NONE

std::tstring VirtualFileSystem::GetUuid(void)
{
	UUID			uuid;				// Folder UUID 
	RPC_TSTR		uuidstr;			// Folder UUID as a string
	RPC_STATUS		rpcstatus;			// Result from RPC function call

	// Create a UUID with the RPC runtime rather than the COM runtime
	rpcstatus = UuidCreate(&uuid);
	if((rpcstatus != RPC_S_OK) && (rpcstatus != RPC_S_UUID_LOCAL_ONLY)) throw Win32Exception(rpcstatus);

	// Convert the UUID into a string
	rpcstatus = UuidToString(&uuid, &uuidstr);
	if(rpcstatus != RPC_S_OK) throw Win32Exception(rpcstatus);

	// Convert the string into a std::tstring and release the RPC buffer
	std::tstring result(reinterpret_cast<tchar_t*>(uuidstr));
	RpcStringFree(&uuidstr);

	return result;
}

// Move me
VfsNode* VirtualFileSystem::CreateFileNode(const CpioFile& cpiofile)
{
	std::tstring localpath = m_tempdir + GetUuid();

	VfsFileNode testme(m_root);

	return nullptr;
}

//-----------------------------------------------------------------------------
// VirtualFileSystem::LoadInitialFileSystem
//
// Loads an initramfs archive into the virtual file system
//
// Arguments:
//
//	path		- Path to the initramfs archive file

void VirtualFileSystem::LoadInitialFileSystem(const tchar_t* path)
{
	// Attempt to open the specified file read-only with sequential scan optimization
	std::unique_ptr<File> archive = File::OpenExisting(path, GENERIC_READ, FILE_SHARE_READ, FILE_FLAG_SEQUENTIAL_SCAN);

	// Decompress as necessary and iterate over all the files contained in the CPIO archive
	CpioArchive::EnumerateFiles(CompressedStreamReader::FromFile(archive), [&](const CpioFile& file) -> void {

		// Depending on the type of node being enumerated, construct the appropriate object
		switch(file.Mode & S_IFMT) {

			case S_IFREG:
				CreateFileNode(file);
				break;

			case S_IFDIR:
				break;

			case S_IFLNK:
				break;

			case S_IFCHR:
				break;

			case S_IFBLK:
				break;

			case S_IFIFO:
				break;

			case S_IFSOCK:
				break;

			default:
				break;
		}
	});
}

//-----------------------------------------------------------------------------

#pragma pop_macro("RPC_TSTR")

#pragma warning(pop)
