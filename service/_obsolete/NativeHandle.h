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

#ifndef __NATIVEHANDLE_H_
#define __NATIVEHANDLE_H_
#pragma once

#include <memory>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// NativeHandle
//
// Wraps a native operating system handle as a shared_ptr

class NativeHandle
{
public:

	// Destructor
	//
	~NativeHandle();

	//-------------------------------------------------------------------------
	// Member Functions

	// FromHandle (static)
	//
	// Creates a NativeHandle shared pointer from an operating system handle
	static std::shared_ptr<NativeHandle> FromHandle(HANDLE handle);

	//-------------------------------------------------------------------------
	// Properties

	// Handle
	//
	// Gets the native handle instance
	__declspec(property(get=getHandle)) HANDLE Handle;
	HANDLE getHandle(void) const;

private:

	NativeHandle(const NativeHandle&)=delete;
	NativeHandle& operator=(const NativeHandle&)=delete;

	// Instance Constructor
	//
	NativeHandle(HANDLE handle);
	friend class std::_Ref_count_obj<NativeHandle>;

	//-------------------------------------------------------------------------
	// Member Variables

	HANDLE					m_handle;			// Contained HANDLE instance
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NATIVEHANDLE_H_
