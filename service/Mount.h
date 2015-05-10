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
#include "FileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Mount
//
// Represents a namespace mount point

class Mount
{
public:

	// Destructor
	//
	~Mount()=default;

	//-------------------------------------------------------------------------
	// Properties

	// Source
	//
	// Retrieves the source device name for the mount point
	__declspec(property(get=getSource)) const char_t* const Source;
	const char_t* const getSource(void) const;

	// Target
	//
	// Gets a reference to the mount point target alias
	__declspec(property(get=getTarget)) std::shared_ptr<FileSystem::Alias> Target;
	std::shared_ptr<FileSystem::Alias> getTarget(void) const;

private:

	Mount(const Mount&)=delete;
	Mount& operator=(const Mount&)=delete;

	// Instance Constructor
	//
	Mount(const char_t* const source, const std::shared_ptr<FileSystem::Alias>& target);
	friend class std::_Ref_count_obj<Mount>;

	//-------------------------------------------------------------------------
	// Member Variables

	const std::string							m_source;	// Mount source device
	const std::shared_ptr<FileSystem::Alias>	m_target;	// Target alias instance
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MOUNT_H_
