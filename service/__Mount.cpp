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
#include "__Mount.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Mount Constructor (protected)
//
// Arguments:
//
//	source		- Source device name of the mount point
//	options		- Reference to the MountOptions instance for this mount

__Mount::__Mount(const char_t* source, std::unique_ptr<MountOptions>&& options) 
	: m_source(source), m_options(std::move(options)) {}

//-----------------------------------------------------------------------------
// __Mount::getOptions
//
// Gets a pointer to the contained MountOptions instance

const MountOptions* __Mount::getOptions(void) const
{
	return m_options.get();
}

//-----------------------------------------------------------------------------
// __Mount::getSource
//
// Gets the source device name for the mount point

const char_t* const __Mount::getSource(void) const
{
	return m_source.c_str();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
