//-----------------------------------------------------------------------------
// Copyright (c) 2014 Michael G. Brehm
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

#ifndef __CPIOARCHIVE_H_
#define __CPIOARCHIVE_H_
#pragma once

#include <functional>
#include <memory>
#include <string>
#include "char_t.h"
#include "StreamReader.h"

#pragma warning(push, 4)			// Enable maximum compiler warnings

// cpio_header_t
//
// Linux initramfs CPIO archive entry header structure
struct cpio_header_t {

	char_t		c_magic[6];				// The string "070701" or "070702"
	char_t		c_ino[8];				// File inode number
	char_t		c_mode[8];				// File mode and permissions
	char_t		c_uid[8];				// File uid
	char_t		c_gid[8];				// File gid
	char_t		c_nlink[8];				// Number of links
	char_t		c_mtime[8];				// Modification time
	char_t		c_filesize[8];			// Size of data field
	char_t		c_maj[8];				// Major part of file device number
	char_t		c_min[8];				// Minor part of file device number
	char_t		c_rmaj[8];				// Major part of device node reference
	char_t		c_rmin[8];				// Minor part of device node reference
	char_t		c_namesize[8];			// Length of filename, including final \0
	char_t		c_chksum[8];			// Checksum of data field if c_magic is 070702
};

//-----------------------------------------------------------------------------
// CpioFile
//
// Entry returned when enumerating the contents of a CPIO archive

class CpioFile
{
public:

	// Instance Constructor
	//
	CpioFile(const cpio_header_t& header, const char_t* path, StreamReader& data);

	//-------------------------------------------------------------------------
	// Properties

	// Data
	//
	// Accesses the embedded file stream reader
	__declspec(property(get=getData)) StreamReader& Data;
	StreamReader& getData(void) const { return m_data; }

	// DeviceMajorVersion
	//
	// Gets the file device major version
	__declspec(property(get=getDeviceMajorVersion)) uint32_t DeviceMajorVersion;
	uint32_t getDeviceMajorVersion(void) const { return m_devmajor; }

	// DeviceMinorVersion
	//
	// Gets the file device minor version
	__declspec(property(get=getDeviceMinorVersion)) uint32_t DeviceMinorVersion;
	uint32_t getDeviceMinorVersion(void) const { return m_devminor; }

	// GID
	//
	// Gets the file owner GID
	__declspec(property(get=getGID)) uint32_t GID;
	uint32_t getGID(void) const { return m_gid; }

	// INode
	//
	// Gets the file inode number
	__declspec(property(get=getINode)) uint32_t INode;
	uint32_t getINode(void) const { return m_inode; }

	// Mode
	//
	// Gets the file mode and permission flags
	__declspec(property(get=getMode)) uint32_t Mode;
	uint32_t getMode(void) const { return m_mode; }

	// ModificationTime
	//
	// Gets the file modification time
	__declspec(property(get=getModificationTime)) uint32_t ModificationTime;
	uint32_t getModificationTime(void) const { return m_mtime; }

	// NumLinks
	//
	// Gets the number of links to this file
	__declspec(property(get=getNumLinks)) uint32_t NumLinks;
	uint32_t getNumLinks(void) const { return m_numlinks; }

	// Path
	//
	// Gets the path of the file (ANSI)
	__declspec(property(get=getPath)) const char_t* Path;
	const char_t* getPath(void) const { return m_path.c_str(); }

	// UID
	//
	// Gets the file owner UID
	__declspec(property(get=getUID)) uint32_t UID;
	uint32_t getUID(void) const { return m_uid; }

private:

	CpioFile(const CpioFile& rhs);
	CpioFile& operator=(const CpioFile& rhs);

	//-------------------------------------------------------------------------
	// Member Variables

	uint32_t						m_inode;			// File inode number
	uint32_t						m_mode;				// File mode and permissions
	uint32_t						m_uid;				// File owner uid
	uint32_t						m_gid;				// File owner gid
	uint32_t						m_numlinks;			// Number of links to this file
	uint32_t						m_mtime;			// Modification time of the file
	uint32_t						m_devmajor;			// File device major version
	uint32_t						m_devminor;			// File device minor version
	std::string						m_path;				// File path
	StreamReader&					m_data;				// File data
};

//-----------------------------------------------------------------------------
// CpioArchive
//
// initramfs CPIO archive reader (Documentation/early-userspace/buffer-format.txt)
//
// This is intended to be used by opening the CPIO[.GZ] archive with an 
// appropriate StreamReader and then passing that into the .EnumerateFiles method 
// along with a lambda to process the data:
//
// std::unique_ptr<StreamReader> input(new GzipStreamReader(...));
// CpioArchive::EnumerateFiles(input, [](const CpioFile& file) -> void {
//
//		create_file(file.Path);
//		while(file.Read(....)) {
//		}
// });

class CpioArchive
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// EnumerateFiles
	//
	// Enumerates over all of the entries in a CPIO archive stream
	static void EnumerateFiles(const std::unique_ptr<StreamReader>& reader, std::function<void(const CpioFile&)> func);

	static void EnumerateFiles(std::unique_ptr<StreamReader>&& reader, std::function<void(const CpioFile&)> func)
		{ return EnumerateFiles(std::forward<const std::unique_ptr<StreamReader>&>(reader), func); }

private:

	CpioArchive();
	CpioArchive(const CpioArchive& rhs);
	CpioArchive& operator=(const CpioArchive& rhs);

	// CpioArchive::FileStream
	//
	// Implements a stream reader for the CPIO file data
	class FileStream : public StreamReader
	{
	public:

		// Instance Constructor
		//
		FileStream(const std::unique_ptr<StreamReader>& basestream, uint32_t length) :
			m_basestream(basestream), m_length(length) {}

		// Destructor
		//
		virtual ~FileStream() {}

		//-------------------------------------------------------------------------
		// Member Functions

		// StreamReader::Read
		//
		// Reads the specified number of bytes from the underlying stream
		virtual uint32_t Read(void* buffer, uint32_t length);

		// StreamReader::Reset
		// NOT IMPLEMENTED
		//
		// Resets the stream back to the beginning
		virtual void Reset(void) { throw Exception(E_NOTIMPL); }

		// StreamReader::Seek
		// NOT IMPLEMENTED
		//
		// Advances the stream to the specified position
		virtual void Seek(uint32_t position) { UNREFERENCED_PARAMETER(position); throw Exception(E_NOTIMPL); }

		//-------------------------------------------------------------------------
		// Properties

		// StreamReader::Length
		//
		// Gets the overall length of the stream
		__declspec(property(get=getLength)) uint32_t Length;
		virtual uint32_t getLength(void) const { return m_length; }

		// StreamReader::Position
		//
		// Gets the current position within the stream
		__declspec(property(get=getPosition)) uint32_t Position;
		virtual uint32_t getPosition(void) { return m_position; }

	private:

		FileStream(const FileStream& rhs);
		FileStream& operator=(const FileStream& rhs);

		//-------------------------------------------------------------------------
		// Member Variables

		// Stream
		const std::unique_ptr<StreamReader>&	m_basestream;		// Base stream object
		uint32_t								m_length;			// Stream length
		uint32_t								m_position = 0;		// Current position
	};
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __CPIOARCHIVE_H_
