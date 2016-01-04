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

#ifndef __NATIVEHOST_H_
#define __NATIVEHOST_H_
#pragma once

#include <memory>
#include <tuple>
#include "Architecture.h"

#pragma warning(push, 4)

// Forward Declarations
//
class NativeProcess;
class NativeThread;

//-----------------------------------------------------------------------------
// Class NativeHost
//
// Wrapper around creation of a native operating system process, the result of
// which is a NativeProcess/NativeThread tuple that allows the two instances
// to be separated and treated as individual entities

class NativeHost
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new NativeProcess/NativeThread instance pair
	static std::tuple<std::unique_ptr<NativeProcess>, std::unique_ptr<NativeThread>> Create(const tchar_t* path, const tchar_t* arguments);
	static std::tuple<std::unique_ptr<NativeProcess>, std::unique_ptr<NativeThread>> Create(const tchar_t* path, const tchar_t* arguments, HANDLE handles[], size_t numhandles);

private:

	NativeHost()=delete;
	~NativeHost()=delete;
	NativeHost(NativeHost const&)=delete;
	NativeHost& operator=(NativeHost const&)=delete;

	//-------------------------------------------------------------------------
	// Private Member Functions
	
	// GetProcessArchitecture (static)
	//
	// Determines the Architecture of a native process
	static enum class Architecture GetProcessArchitecture(HANDLE process);
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NATIVEHOST_H_

