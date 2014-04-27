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

#include "stdafx.h"						// Include project pre-compiled headers
#include "VfsFileNode.h"				// Include VfsFileNode declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VfsFileNode Constructor
//
// Arguments:
//
//	mode		- Initial mode flags for the virtual file
//	uid			- Initial owner uid for the virtual file
//	gid			- Initial owner gid for the virtual file

VfsFileNode::VfsFileNode(mode_t mode, uid_t uid, gid_t gid) : VfsNode(mode, uid, gid) 
{
	_ASSERTE((mode & S_IFMT) == S_IFREG);
	if((mode & S_IFMT) != S_IFREG) throw Exception(E_VFS_INVALIDNODEMODE, mode);

	// Generate the underlying file name for this node in the temporary folder
	std::tstring filename = VfsNode::GenerateTemporaryFileName();

	// Create a temporary, delete-on-close file as backing storage for the node data
	m_handle = CreateFile(filename.c_str(), FILE_ALL_ACCESS, 0, nullptr, CREATE_NEW, 
		FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
	if(m_handle == INVALID_HANDLE_VALUE) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// VfsFileNode Constructor
//
// Arguments:
//
//	mode		- Initial mode flags for the virtual file
//	uid			- Initial owner uid for the virtual file
//	gid			- Initial owner gid for the virtual file
//	data		- Initial data stream for the virtual file

VfsFileNode::VfsFileNode(mode_t mode, uid_t uid, gid_t gid, StreamReader& data) 
	: VfsFileNode(mode, uid, gid) 
{
	const int BUFFER_SIZE = (64 KiB);				// Local file buffer size

	// Above constructor should have been successfully invoked before we get here
	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);

	// Use a 64KiB buffer to read from the stream into the target file
	uint8_t* buffer = new uint8_t[BUFFER_SIZE];
	if(!buffer) throw Exception(E_OUTOFMEMORY);

	try {

		// Load the file from the provided stream in BUFFER_SIZE chunks
		uint32_t bytesread = data.Read(buffer, BUFFER_SIZE);
		while(bytesread > 0) {

			if(!WriteFile(m_handle, buffer, bytesread, reinterpret_cast<LPDWORD>(&bytesread), nullptr)) throw Win32Exception();
			bytesread = data.Read(buffer, BUFFER_SIZE);
		}
	} 
	
	catch(...) { delete[] buffer; throw; }
	delete[] buffer;

	// Reset the file pointer back to the beginning of the file
	SetFilePointer(m_handle, 0, nullptr, FILE_BEGIN);
}

//-----------------------------------------------------------------------------
// VfsFileNode Destructor

VfsFileNode::~VfsFileNode()
{
	// Close the underlying temporary file handle
	CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
