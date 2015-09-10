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

#include "stdafx.h"
#include "Host.h"

#include "NativeProcess.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Host Constructor
//
// Arguments:
//
//	nativeproc		- NativeProcess instance to take ownership of

Host::Host(std::unique_ptr<NativeProcess> nativeproc) : m_nativeproc(std::move(nativeproc))
{
}

//-----------------------------------------------------------------------------
// Host Destructor

Host::~Host()
{
}

//------------------------------------------------------------------------------
// Host::getArchitecture
//
// Gets the architecture of the native hosting process

enum class Architecture Host::getArchitecture(void) const
{
	return m_nativeproc->Architecture;
}

//-----------------------------------------------------------------------------
// Host::FromNativeProcess (static)
//
// Creates a new Host instance from an existing NativeProcess instance
//
// Arguments:
//
//	nativeproc		- NativeProcess instance to take ownership of

std::unique_ptr<Host> Host::FromNativeProcess(std::unique_ptr<NativeProcess> nativeproc)
{
	return std::make_unique<Host>(std::move(nativeproc));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
