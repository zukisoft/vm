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

#ifndef __NEWROOTFILESYSTEM_H_
#define __NEWROOTFILESYSTEM_H_
#pragma once

#include <memory>
#include "LinuxException.h"
#include "FileSystem.h"
#include "MountOptions.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem

class NewRootFileSystem
{
public:

	// Destructor
	//
	virtual ~NewRootFileSystem()=default;

	// Mount (static)
	//
	// Mounts the file system
	static std::shared_ptr<FileSystem::Mount> Mount(const char_t* const source, uint32_t flags, const void* data, size_t datalen);

private:

	NewRootFileSystem(const NewRootFileSystem&)=delete;
	NewRootFileSystem& operator=(const NewRootFileSystem&)=delete;

	//
	// RootFileSystem::Mount
	//

	class Mount : public FileSystem::Mount
	{
	friend class NewRootFileSystem;
	public:

		// Duplicate
		//
		// Duplicates this mount instance
		virtual std::shared_ptr<FileSystem::Mount> Duplicate(void) const;

		// Remount
		//
		// Remounts this mount point with different flags and arguments
		virtual void Remount(uint32_t flags, const void* data, size_t datalen);

		// Options
		//
		// Gets a pointer to the contained MountOptions instance
		__declspec(property(get=getOptions)) const MountOptions* Options;
		virtual const MountOptions* getOptions(void) const;

		// Root
		//
		// Gets a pointer to the root node for this mount point
		__declspec(property(get=getRoot)) std::shared_ptr<FileSystem::Node> Root;
		virtual std::shared_ptr<FileSystem::Node> getRoot(void) const;

		// Source
		//
		// Retrieves the source device name for the mount point
		__declspec(property(get=getSource)) const char_t* Source;
		virtual const char_t* getSource(void) const;

	private:

		Mount(const Mount&)=delete;
		Mount& operator=(const Mount&)=delete;

		// Instance Constructor
		//
		Mount(const char_t* source, std::unique_ptr<MountOptions>&& options);
		friend class std::_Ref_count_obj<Mount>;

		// Member Variables
		//
		std::string							m_source;		// Source device
		const std::unique_ptr<MountOptions> m_options;		// Mounting options
	};

	// Instance Constructor
	//
	NewRootFileSystem()=default;
	friend class std::_Ref_count_obj<NewRootFileSystem>;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NEWROOTFILESYSTEM_H_