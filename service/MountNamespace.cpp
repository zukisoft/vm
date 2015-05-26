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
#include "MountNamespace.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// MountNamespace Constructor (private)
//
// Arguments:
//
//	mounts		- Collection of mounts to contain upon construction

MountNamespace::MountNamespace(mount_set_t&& mounts) : m_mounts(std::move(mounts)) {}

//-----------------------------------------------------------------------------
// MountNamespace::Create (static)
//
// Creates a new MountNamespace instance
//
// Arguments:
//
//	NONE

std::shared_ptr<MountNamespace> MountNamespace::Create(void)
{
	// Create a new empty MountNamespace instance
	return std::make_shared<MountNamespace>(mount_set_t());
}

//-----------------------------------------------------------------------------
// MountNamespace::Create (static)
//
// Creates a clone of an existing MountNamespace instance
//
// Arguments:
//
//	mountns		- Existing MountNamespace to be copied

std::shared_ptr<MountNamespace> MountNamespace::Create(const std::shared_ptr<MountNamespace>& mountns)
{
	mount_set_t				mounts;			// New collection of mounts

	// Iterate over the existing collection of mounts and copy each of them
	for(const auto& iterator : mountns->m_mounts) {

		// CREATE NEW MOUNT
		mounts.emplace(nullptr, nullptr);

		// ADD NEW NAMESPACE/MOUNT to existing target Alias
		// ?? I don't have the parent namespace instance here!!!
	}

	return std::make_shared<MountNamespace>(mount_set_t());
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
