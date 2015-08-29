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

#ifndef __TEMPFILESYSTEM_H_
#define __TEMPFILESYSTEM_H_
#pragma once

#include <algorithm>
#include <memory>
#include "Capabilities.h"
#include "FileSystem.h"
#include "IndexPool.h"
#include "LinuxException.h"
#include "MountOptions.h"
#include "SystemInformation.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TempFileSystem
//
// words
//
// Rather than using a virtual block device constructed on virtual memory, this
// uses a private Windows heap to store the file system data.  There are a number
// of challenges with the virtual block device method that can be easily overcome
// by doing it this way; let the operating system do the heavy lifting
//
// Extended Mount Options (see mount(8) - "Mount options for tmpfs") 
//
//	size		=nnn[K|k|M|m|G|g|%] *
//	nr_blocks	=nnn[K|k|M|m|G|g|%] *
//	nr_inodes	=nnn[K|k|M|m|G|g|%] *
//	mode		=nnn
//	uid			=nnn
//	gid			=nnn
//
//  * - option may also be specified during a remount operation

class TempFileSystem //: public FileSystem
{
public:

	// Destructor
	//
	~TempFileSystem();

	// Mount (static)
	//
	// Creates an instance of the file system
	static std::shared_ptr<FileSystem::Mount> Mount(const char_t* source, uint32_t flags, const void* data, size_t datalength);

	// Stat
	//
	// Provides statistical information about the file system
	virtual void Stat(uapi::statfs* stats);
		
	//-------------------------------------------------------------------------
	// Properties

	// MaximumNodes
	//
	// Gets/sets the maximum number of nodes allowed in the file system
	__declspec(property(get=getMaximumNodes, put=putMaximumNodes)) size_t MaximumNodes;
	size_t getMaximumNodes(void) const;
	void putMaximumNodes(size_t value);

	// MaximumSize
	//
	// Gets/sets the maximum size of the file system
	__declspec(property(get=getMaximumSize, put=putMaximumSize)) size_t MaximumSize;
	size_t getMaximumSize(void) const;
	void putMaximumSize(size_t value);

	// ReadOnly
	// todo: this is dumb, make a SetFlags() or something instead otherwise
	// you end up with 100 of these annoying things
	//
	// Gets/sets the read-only attribute of the file system
	__declspec(property(get=getReadOnly, put=putReadOnly)) bool ReadOnly;
	bool getReadOnly(void) const;
	void putReadOnly(bool value);

private:

	TempFileSystem(const TempFileSystem&)=delete;
	TempFileSystem& operator=(const TempFileSystem&)=delete;

	// TempFileSystem::Mount
	//
	class Mount : public FileSystem::Mount
	{
	public:

		// Destructor
		//
		~Mount()=default;

		// Create (static)
		//
		// Creates a Mount instance
		static std::shared_ptr<Mount> Create(const std::shared_ptr<TempFileSystem>& fs, uint32_t flags);

		// Duplicate (FileSystem::Mount)
		//
		// Duplicates this mount instance
		virtual std::shared_ptr<FileSystem::Mount> Duplicate(void);

		// Remount (FileSystem::Mount)
		//
		// Remounts this mount point with different flags and arguments
		virtual void Remount(uint32_t flags, const void* data, size_t datalen);

		// Stat (FileSystem::Mount)
		//
		// Provides statistical information about the mounted file system
		virtual void Stat(uapi::statfs* stats);

		// getFlags (FileSystem::Mount)
		//
		// Gets the flags set on this mount
		virtual uint32_t getFlags(void);

		virtual std::shared_ptr<FileSystem::Node> getRoot(void) { return nullptr; } // todo - move to CPP

		virtual const char_t* getSource(void) { return nullptr; }

	private:

		Mount(const Mount&)=delete;
		Mount& operator=(const Mount&)=delete;

		// Instance Constructor
		//
		Mount(const std::shared_ptr<TempFileSystem>& fs, uint32_t flags);
		friend class std::_Ref_count_obj<Mount>;

		// Member Variables
		//
		const std::shared_ptr<TempFileSystem>	m_fs;		// File system instance
		uint32_t								m_flags;	// Mounting flags
	};

	// Instance Constructor
	//
	TempFileSystem(const char_t* source, size_t maxsize, size_t maxnodes);
	friend class std::_Ref_count_obj<TempFileSystem>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ParseScaledInteger (static)
	//
	// Parses a scaled integer mounting option (size, nr_blocks, nr_inodes)
	static size_t ParseScaledInteger(const std::string& value, size_t maximum);

	//-------------------------------------------------------------------------
	// Member Variables

	const std::string			m_source;			// Source device string
	HANDLE						m_heap;				// Private heap handle
	size_t						m_size;				// Current file system size
	size_t						m_maxsize;			// Maximum file system size
	size_t						m_nodes;			// Current number of nodes
	size_t						m_maxnodes;			// Maximum number of nodes
	IndexPool<intptr_t>			m_indexpool;		// Pool of node index numbers
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TEMPFILESYSTEM_H_