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

#ifndef __FILEDESCRIPTOR_H_
#define __FILEDESCRIPTOR_H_

#include "FsObject.h"					// Include FsObject class declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// FileDescriptor
//
// Object stored in the file descriptor table.  This is envisioned as a subset
// of the FsObject class, containing only the underlying handle type and value
// Automatic release/unwind of the contained handles is not provided.

class FileDescriptor
{
public:

	// Instance Constructor
	//
	FileDescriptor(const FsObject& object) : FileDescriptor(object, INVALID_HANDLE_VALUE) {}
	FileDescriptor(const FsObject& object, HANDLE handle) : m_type(object.objecttype), 
		m_fshandle(object.fshandle), m_oshandle(handle) {}

	// Copy Constructor
	//
	FileDescriptor(const FileDescriptor& rhs) : m_type(rhs.m_type), m_fshandle(rhs.m_fshandle), 
		m_oshandle(rhs.m_oshandle) {}

	//-------------------------------------------------------------------------
	// Overloaded Operators

	// fshandle_t conversion
	//
	operator fshandle_t() const { return m_fshandle; }

	// HANDLE conversion
	//
	operator HANDLE() const { return m_oshandle; }

	// Assignment operator
	//
	FileDescriptor& operator=(const FileDescriptor& rhs)
	{
		m_type = rhs.m_type;
		m_fshandle = rhs.m_fshandle;
		m_oshandle = rhs.m_oshandle;

		return *this;
	}

	bool operator==(const FileDescriptor& rhs)
	{
		return ((m_type == rhs.m_type) && (m_fshandle == rhs.m_fshandle) &&
			(m_oshandle == rhs.m_oshandle));
	}

	//-------------------------------------------------------------------------
	// Properties

	// FsHandle
	//
	// Gets/Sets the contained fshandle_t
	__declspec(property(get=getFsHandle, put=putFsHandle)) fshandle_t FsHandle;
	fshandle_t getFsHandle(void) const { return m_fshandle; }
	void putFsHandle(fshandle_t value) { m_fshandle = value; }

	// OsHandle
	//
	// Gets/Sets the contained operating system handle
	__declspec(property(get=getOsHandle, put=putOsHandle)) HANDLE OsHandle;
	HANDLE getOsHandle(void) const { return m_oshandle; }
	void putOsHandle(HANDLE value) { m_oshandle = value; }

	// Type
	//
	// Gets/Sets the file descriptor type flags
	__declspec(property(get=getType, put=putType)) fsobjecttype_t Type;
	fsobjecttype_t getType(void) const { return m_type; }
	void putType(fsobjecttype_t value) { m_type = value; }

	//__declspec(property(get=getNull)) static FileDescriptor& Null;
	//static FileDescriptor& getNull(void) { return s_null; }
	
	static const FileDescriptor Null;

private:

	FileDescriptor() : m_type(static_cast<fsobjecttype_t>(0)), m_fshandle(0), m_oshandle(INVALID_HANDLE_VALUE) {}

	//-------------------------------------------------------------------------
	// Member Variables

	fsobjecttype_t			m_type;				// Type of contained handle
	fshandle_t				m_fshandle;			// Remote services handle
	HANDLE					m_oshandle;			// Operating system handle

	//static FileDescriptor	s_null;
};

__declspec(selectany) const FileDescriptor FileDescriptor::Null;

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILEDESCRIPTOR_H_
