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
#include "TempFileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TempFileSystem Constructor (private)
//
// Arguments:
//
//	source		- Source device name, as provided to Mount()
//	maxsize		- Maximum size of the file system
//	maxinodes	- Maximum number of nodes that can be created

TempFileSystem::TempFileSystem(const char_t* source, size_t maxsize, size_t maxnodes) : m_source(source ? source : "")
{
	// Create a private heap to contain the file system data.  Do not specify a maximum
	// size here, it limits what can be allocated and cannot be changed after the fact
	m_heap = HeapCreate(0, 0, 0);
	if(m_heap == nullptr) throw Win32Exception();

	// Initialize size is zero, maximum is whatever was requested
	m_size = 0;
	m_maxsize = maxsize;

	// Initialize node count is zero, maximum is whatever was requested
	m_nodes = 0;
	m_maxnodes = maxnodes;
}

//-----------------------------------------------------------------------------
// TempFileSystem Destructor

TempFileSystem::~TempFileSystem()
{
	if(m_heap) HeapDestroy(m_heap);
}

//-----------------------------------------------------------------------------
// TempFileSystem::getMaximumNodes
//
// Gets the maximum number of nodes that can be created in the file system

size_t TempFileSystem::getMaximumNodes(void) const
{
	// todo: synchronization object
	return m_maxnodes;
}

//-----------------------------------------------------------------------------
// TempFileSystem::putMaximumNodes
//
// Sets the maximum number of nodes that can be created in the file system

void TempFileSystem::putMaximumNodes(size_t value)
{
	// todo: synchronization object
	if(m_nodes > value) throw LinuxException(LINUX_EINVAL);
	m_maxnodes = value;
}

//-----------------------------------------------------------------------------
// TempFileSystem::getMaximumSize
//
// Gets the maximum size of the file system, in bytes

size_t TempFileSystem::getMaximumSize(void) const
{
	// todo: synchronization object
	return m_maxsize;
}

//-----------------------------------------------------------------------------
// TempFileSystem::putMaximumSize
//
// Sets the maximum size of the file system, in bytes

void TempFileSystem::putMaximumSize(size_t value)
{
	// todo: synchronization object
	if(m_size > value) throw LinuxException(LINUX_EINVAL);
	m_maxsize = value;
}

//-----------------------------------------------------------------------------
// TempFileSystem::Mount (static)
//
// Creates a new temporary file system instance
//
// Arguments:
//
//	source		- Source device string
//	flags		- Standard mounting option flags
//	data		- Extended/custom mounting options
//	datalength	- Length of the extended mounting options data

std::shared_ptr<FileSystem::Mount> TempFileSystem::Mount(const char_t* source, uint32_t flags, const void* data, size_t datalength)
{
	if(source == nullptr) throw LinuxException(LINUX_EFAULT);

	Capabilities::Demand(Capability::SystemAdmin);

	// Parse the provided mounting options
	MountOptions options(flags, data, datalength);

	// Determine the maximum size of the memory available, used when scaling size/nodes to a percentage
	uint64_t maxaccessible = std::min(SystemInformation::TotalPhysicalMemory, SystemInformation::TotalVirtualMemory);
	size_t memsize = static_cast<size_t>(std::min(maxaccessible, static_cast<uint64_t>(MAXSIZE_T)));

	// The default file system and node maximums are half of the absolute maximums
	size_t size = align::up((memsize >> 1), SystemInformation::PageSize);
	size_t nodes = size / SystemInformation::PageSize;

	// size=
	//
	// Sets the maximum size of the file system in bytes
	if(options.Arguments.Contains("size"))
		size = align::up(ParseScaledInteger(options.Arguments["size"], memsize), SystemInformation::PageSize);

	// nr_blocks=
	//
	// Sets the maximum size of the file system in blocks (page size)
	if(options.Arguments.Contains("nr_blocks")) 
		size = ParseScaledInteger(options.Arguments["nr_blocks"], memsize / SystemInformation::PageSize) * SystemInformation::PageSize;

	// nr_inodes=
	//
	// Sets the maximum number of nodes that can be created
	if(options.Arguments.Contains("nr_inodes"))
		nodes = ParseScaledInteger(options.Arguments["nr_inodes"], memsize / SystemInformation::PageSize);

	// mode=
	//
	// Sets the permission flags to apply to the root directory
	if(options.Arguments.Contains("mode")) {
		// todo
	}

	// uid=
	//
	// Sets the owner UID to apply to the root directory
	if(options.Arguments.Contains("uid")) {
		// todo
	}

	// gid=
	//
	// Sets the owner GID to apply to the root directory
	if(options.Arguments.Contains("gid")) {
		// todo
	}

	// todo: need the root directory node
	return Mount::Create(std::make_shared<TempFileSystem>(source, size, nodes), options.Flags);
}

//-----------------------------------------------------------------------------
// TempFileSystem::ParseScaledInteger (private, static)
//
// Parses the value of a scaled integer mountint option (size, nr_blocks, nr_inodes)
//
// Arguments:
//
//	value		- Integer string with the optional magnitude/percentage suffix
//	maximum		- Absolute maximum value to use in conjunction with a percentage suffix

size_t TempFileSystem::ParseScaledInteger(const std::string& value, size_t maximum)
{
	uint64_t result = 0;	// Use a 64-bit unsigned integer for the calculations

	try {

		// Check the string for a suffix that determines the magnitude of
		// the base value, if it's a percentage of the maximum, or a raw number
		switch(value.back()) {

			// K - value is in kibi (* 1024)
			case 'K':
			case 'k':
				result = std::stoull(value.substr(0, value.length() - 1), 0, 0) KiB;
				break;

			// M - value is in mibi (* 1048576)
			case 'M':
			case 'm':
				result = std::stoull(value.substr(0, value.length() - 1), 0, 0) MiB;
				break;

			// G - value is in gibi (* 1073741824)
			case 'G':
			case 'g':
				result = std::stoull(value.substr(0, value.length() - 1), 0, 0) GiB;
				break;

			// % - value is a percentage of the provided maximum
			case '%':

				// the provided maximum may not be able to be multiplied without overflow, so
				// convert the percentage value into a double and calculate it with just division
				result = static_cast<uint64_t>(maximum * (static_cast<double>(std::stoull(value.substr(0, value.length() - 1), 0, 0)) / 100));
				break;

			// no suffix - value is a raw number
			default:
				result = std::stoull(value, 0, 0);
				break;
		}

		// Caller expects a size_t, on x86 the calculated result must be checked for overflow
		if(result > MAXSIZE_T) throw LinuxException(LINUX_EINVAL);
		return static_cast<size_t>(result);
	}

	// Translate all exceptions into EINVAL to indicate the input was no good
	catch(...) { throw LinuxException(LINUX_EINVAL); }
}

void TempFileSystem::putReadOnly(bool value)
{
	(value);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Stat
//
// Provides statistical information about the file system
//
// Arguments:
//
//	stats		- Structure to receieve the file system statistics

void TempFileSystem::Stat(uapi::statfs* stats)
{
	if(stats = nullptr) throw LinuxException(LINUX_EFAULT);

	// Initialize the output statfs structure to all zeros
	memset(stats, 0, sizeof(uapi::statfs));

	// todo: set these properly
	stats->f_type = LINUX_TMPFS_MAGIC;
	stats->f_bsize = SystemInformation::PageSize;
	stats->f_blocks = m_maxsize / 512;
	stats->f_bfree = (m_maxsize - m_size) / 512;
	stats->f_bavail = stats->f_bfree;
	stats->f_files = m_maxnodes;
	stats->f_ffree = (m_maxnodes - m_nodes);
	//stats->f_fsid = 0;
	stats->f_namelen = 255;
	stats->f_frsize = 0;
	stats->f_flags = 0;		// this should exclude the per-mount flags
}

//
// TEMPFILESYSTEM::MOUNT
//

//-----------------------------------------------------------------------------
// TempFileSystem::Mount Constructor
//
// Arguments:
//
//	fs		- Reference to the TempFileSystem instance
//	flags	- Standard mounting option flags

TempFileSystem::Mount::Mount(const std::shared_ptr<TempFileSystem>& fs, uint32_t flags) : m_fs(fs), m_flags(flags)
{
}

//-----------------------------------------------------------------------------
// TempFileSystem::Mount::Create (static)
//
// Creates a new Mount instance
//
// Arguments:
//
//	fs		- Reference to the TempFileSystem instance to mount
//	flags	- Standard mounting flags and options

std::shared_ptr<class TempFileSystem::Mount> TempFileSystem::Mount::Create(const std::shared_ptr<TempFileSystem>& fs, uint32_t flags)
{
	return std::make_shared<class TempFileSystem::Mount>(fs, flags);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Mount::Duplicate
//
// Duplicates the mount instance
//
// Arguments:
//
//	NONE

std::shared_ptr<FileSystem::Mount> TempFileSystem::Mount::Duplicate(void)
{
	return std::make_shared<class TempFileSystem::Mount>(m_fs, m_flags);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Mount::Remount
//
// Remounts the file system with different options
//
// Arguments:
//
//	flags		- Standard mounting option flags
//	data		- Extended/custom mounting options
//	datalength	- Length of the extended mounting options data

void TempFileSystem::Mount::Remount(uint32_t flags, const void* data, size_t datalen)
{
	Capabilities::Demand(Capability::SystemAdmin);

	// MS_REMOUNT must be specified in the flags when calling this function
	if((flags & LINUX_MS_REMOUNT) != LINUX_MS_REMOUNT) throw LinuxException(LINUX_EINVAL);

	// Parse the provided mounting options into remount flags and key/value pairs
	MountOptions options(flags & LINUX_MS_RMT_MASK, data, datalen);

	// Filter the flags to only those options which have changed from the current ones
	uint32_t changedflags = m_flags ^ options.Flags;

	// Determine the maximum size of the memory available, used when scaling size/nodes to a percentage
	uint64_t maxaccessible = std::min(SystemInformation::TotalPhysicalMemory, SystemInformation::TotalVirtualMemory);
	size_t memsize = static_cast<size_t>(std::min(maxaccessible, static_cast<uint64_t>(MAXSIZE_T)));

	// size=
	//
	if(options.Arguments.Contains("size"))
		m_fs->MaximumSize = ParseScaledInteger(options["size"], memsize);

	// nr_blocks=
	//
	if(options.Arguments.Contains("nr_blocks"))
		m_fs->MaximumSize = ParseScaledInteger(options["nr_blocks"], memsize / SystemInformation::PageSize) * SystemInformation::PageSize;

	// nr_inodes=
	//
	if(options.Arguments.Contains("nr_inodes"))
		m_fs->MaximumNodes = ParseScaledInteger(options["nr_inodes"], memsize / SystemInformation::PageSize);

	// MS_RDONLY
	//
	//TODO PUT ME BACK if(changedflags & LINUX_MS_RDONLY) m_fs->ReadOnly = (options[LINUX_MS_RDONLY]);

	// MS_SYNCHRONOUS
	//
	if(changedflags & LINUX_MS_SYNCHRONOUS) {

		// note: options[flag] = new value
	}

	// MS_MANDLOCK
	//
	if(changedflags & LINUX_MS_SYNCHRONOUS) {

		// note: options[flag] = new value
	}

	// MS_I_VERSION
	//
	if(changedflags & LINUX_MS_I_VERSION) {

		// note: options[flag] = new value
	}

	// MS_LAZYTIME
	//
	if(changedflags & LINUX_MS_LAZYTIME) {

		// note: options[flag] = new value
	}
}

//-----------------------------------------------------------------------------
// TempFileSystem::Mount::getFlags
//
// Gets the standard mount option flags

uint32_t TempFileSystem::Mount::getFlags(void)
{
	return m_flags;
}

//-----------------------------------------------------------------------------
// TempFileSystem::Mount::Stat
//
// Provides statistical information about the mounted file system
//
// Arguments:
//
//	stats		- Structure to receieve the file system statistics

void TempFileSystem::Mount::Stat(uapi::statfs* stats)
{
	if(!stats) throw LinuxException(LINUX_EFAULT);

	// Call through to the underlying file system for general statistics
	m_fs->Stat(stats);

	// Combine the per-mount flags defined on this mount with the flags
	// provided by the underlying file system
	stats->f_flags |= (m_flags & LINUX_MS_PERMOUNT_MASK);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
