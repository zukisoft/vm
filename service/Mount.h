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

#ifndef __MOUNT_H_
#define __MOUNT_H_
#pragma once

#include <memory>
#include <string>
#include "Alias.h"
#include "MountOptions.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Mount (abstract)
//
// Implements a file system mount point

class Mount
{
public:

	// Destructor
	//
	virtual ~Mount()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Duplicate
	//
	// Duplicates this mount instance with same flags and extra arguments
	virtual std::shared_ptr<Mount> Duplicate(void) = 0;

	// Remount
	//
	// Remounts this mount point with different flags and arguments
	virtual void Remount(uint32_t flags, const void* data, size_t datalen) = 0;

	//-------------------------------------------------------------------------
	// Properties

	// Options
	//
	// Gets a pointer to the contained MountOptions instance
	__declspec(property(get=getOptions)) const MountOptions* Options;
	const MountOptions* getOptions(void) const;

	// Root
	//
	// Gets a pointer to the root alias for this mount point
	__declspec(property(get=getRoot)) std::shared_ptr<Alias> Root;
	virtual std::shared_ptr<Alias> getRoot(void) const = 0;

	// Source
	//
	// Retrieves the source device name for the mount point
	__declspec(property(get=getSource)) const char_t* const Source;
	const char_t* const getSource(void) const;

protected:

	// Instance Constructor
	//
	Mount(const char_t* source, std::unique_ptr<MountOptions>&& options);

private:

	Mount(const Mount&)=delete;
	Mount& operator=(const Mount&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	const std::string						m_source;	// Source device
	const std::unique_ptr<MountOptions>		m_options;	// Mount options
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MOUNT_H_
