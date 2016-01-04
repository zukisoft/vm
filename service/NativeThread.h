//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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

#ifndef __NATIVETHREAD_H_
#define __NATIVETHREAD_H_
#pragma once

#include "Architecture.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// NativeThread
//
// Owns a native operating system thread handle and abstracts the operations that
// can be performed against that thread

class NativeThread
{
public:

	// Instance Constructor
	//
	NativeThread(enum class Architecture architecture, HANDLE thread, DWORD threadid);

	// Destructor
	//
	~NativeThread();

	//-------------------------------------------------------------------------
	// Member Functions

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture associated with the task state
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	enum class Architecture getArchitecture(void) const;

private:

	NativeThread(NativeThread const&)=delete;
	NativeThread& operator=(NativeThread const&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables
	
	enum class Architecture	const	m_architecture;		// Task architecture
	HANDLE const					m_thread;			// Thread handle
	DWORD const						m_threadid;			// Thread identifier
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NATIVETHREAD_H_
