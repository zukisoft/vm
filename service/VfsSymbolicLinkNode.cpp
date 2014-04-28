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
#include "VfsSymbolicLinkNode.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// VfsSymbolcLinkNode Constructor
//
// Arguments:
//
//	mode		- Initial mode flags for the symbolic link (generally ignored)
//	uid			- Initial owner uid for the symbolic link
//	gid			- Initial owner gid for the symbolic link
//	target		- Target path of the symbolic link

VfsSymbolicLinkNode::VfsSymbolicLinkNode(mode_t mode, uid_t uid, gid_t gid, const char_t* target) 
	: VfsNode(mode | S_IRWXU | S_IRWXG | S_IRWXO, uid, gid), m_target(target)
{
	_ASSERTE((mode & S_IFMT) == S_IFLNK);
	if((mode & S_IFMT) != S_IFLNK) throw Exception(E_VFS_INVALIDNODEMODE, mode);
}

//-----------------------------------------------------------------------------
// VfsSymbolcLinkNode Constructor
//
// Arguments:
//
//	mode		- Initial mode flags for the symbolic link (generally ignored)
//	uid			- Initial owner uid for the symbolic link
//	gid			- Initial owner gid for the symbolic link
//	data		- StreamReader instance containing the target string

VfsSymbolicLinkNode::VfsSymbolicLinkNode(mode_t mode, uid_t uid, gid_t gid, StreamReader& data) 
	: VfsNode(mode | S_IRWXU | S_IRWXG | S_IRWXO, uid, gid)
{
	const int BUFFER_SIZE = (1 KiB);				// Local file buffer size

	_ASSERTE((mode & S_IFMT) == S_IFLNK);
	if((mode & S_IFMT) != S_IFLNK) throw Exception(E_VFS_INVALIDNODEMODE, mode);

	// Use a 1KiB buffer to read from the stream into the target string
	uint8_t* buffer = new uint8_t[BUFFER_SIZE];
	if(!buffer) throw Exception(E_OUTOFMEMORY);

	try {

		// Load the symbolic link target from the provided stream in BUFFER_SIZE chunks
		uint32_t bytesread = data.Read(buffer, BUFFER_SIZE);
		while(bytesread > 0) {

			m_target.append(reinterpret_cast<const char_t*>(buffer), bytesread);
			bytesread = data.Read(buffer, BUFFER_SIZE);
		}
	} 
	
	catch(...) { delete[] buffer; throw; }
	delete[] buffer;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
