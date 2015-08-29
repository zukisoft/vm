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

#ifndef __FILEPERMISSION_H_
#define __FILEPERMISSION_H_
#pragma once

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// FilePermission
//
// todo: words
// TODO: This ultimately needs to know who the calling user is, from TLS or something;
// it will be the same mechanism as Capability

class FilePermission final
{
public:

	// Copy Constructor
	//
	FilePermission(const FilePermission&)=default;

	// Destructor
	//
	~FilePermission()=default;

	// bitwise or operator
	//
	FilePermission operator|(const FilePermission rhs) const;

	//-------------------------------------------------------------------------
	// Fields

	// Execute (static)
	//
	// Defines a FilePermission for execute access
	static const FilePermission Execute;

	// Read (static)
	//
	// Defines a FilePermission for read access
	static const FilePermission Read;

	// Write (static)
	//
	// Defines a FilePermission for write access
	static const FilePermission Write;

	//-------------------------------------------------------------------------
	// Member Functions

	// Check (static)
	//
	// Checks the specified access from permission components
	static bool Check(const FilePermission& permission, uapi::uid_t uid, uapi::gid_t gid, uapi::mode_t mode);

	// Demand (static)
	//
	// Demands the specified access from permission components
	static void Demand(const FilePermission& permission, uapi::uid_t uid, uapi::gid_t gid, uapi::mode_t mode);

private:

	FilePermission& operator=(const FilePermission&)=delete;

	// Instance Constructor
	//
	FilePermission(uint8_t mask);

	//-------------------------------------------------------------------------
	// Constants

	// EXECUTE
	//
	// Access mask for execute access to a file system object
	static const uint8_t EXECUTE = 01;

	// WRITE
	//
	// Access mask for write access to a file system object
	static const uint8_t WRITE = 02;

	// READ
	//
	// Access mask for read access to a file system object
	static const uint8_t READ = 04;

	//-------------------------------------------------------------------------
	// Member Variables

	const uint8_t			m_mask;			// Access mask
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILEPERMISSION_H_
