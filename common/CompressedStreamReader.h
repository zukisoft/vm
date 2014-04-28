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

#ifndef __COMPRESSEDSTREAMREADER_H_
#define __COMPRESSEDSTREAMREADER_H_
#pragma once

#include <memory>
#include "BufferStreamReader.h"
#include "BZip2StreamReader.h"
#include "Exception.h"
#include "File.h"
#include "GZipStreamReader.h"
#include "Lz4StreamReader.h"
#include "LzopStreamReader.h"
#include "MappedFile.h"
#include "MappedFileView.h"
#include "StreamReader.h"
#include "XzStreamReader.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// CompressedStreamReader
//
// Generic compressed data stream reader, the underlying type of the compression
// is automatically detected by examining the data

class CompressedStreamReader : public StreamReader
{
public:

	// Destructor
	//
	virtual ~CompressedStreamReader()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// StreamReader::Read
	//
	// Reads the specified number of bytes from the underlying stream
	virtual uint32_t Read(void* buffer, uint32_t length) { return m_stream->Read(buffer, length); }

	// StreamReader::Reset
	//
	// Resets the stream back to the beginning
	virtual void Reset(void) { m_stream->Reset(); }

	// StreamReader::Seek
	//
	// Advances the stream to the specified position
	virtual void Seek(uint32_t position) { m_stream->Seek(position); }

	// FromFile
	//
	// Creates the compressed stream reader from a file
	static std::unique_ptr<StreamReader> FromFile(const std::unique_ptr<File>& file)
		{ return std::unique_ptr<StreamReader>(new CompressedStreamReader(file, 0, 0)); }
	static std::unique_ptr<StreamReader> FromFile(std::unique_ptr<File>&& file)
		{ return FromFile(std::forward<std::unique_ptr<File>&>(file)); }

	static std::unique_ptr<StreamReader> FromFile(const std::unique_ptr<File>& file, size_t offset)
		{ return std::unique_ptr<StreamReader>(new CompressedStreamReader(file, offset, 0)); }
	static std::unique_ptr<StreamReader> FromFile(std::unique_ptr<File>&& file, size_t offset)
		{ return FromFile(std::forward<std::unique_ptr<File>&>(file), offset); }

	static std::unique_ptr<StreamReader> FromFile(const std::unique_ptr<File>& file, size_t offset, size_t length)
		{ return std::unique_ptr<StreamReader>(new CompressedStreamReader(file, offset, length)); }
	static std::unique_ptr<StreamReader> FromFile(std::unique_ptr<File>&& file, size_t offset, size_t length)
		{ return FromFile(std::forward<std::unique_ptr<File>&>(file), offset, length); }

	//-------------------------------------------------------------------------
	// Properties

	// StreamReader::getPosition
	//
	// Gets the current position within the stream
	virtual uint32_t getPosition(void) { return m_stream->Position; }

private:

	CompressedStreamReader(const CompressedStreamReader&)=delete;
	CompressedStreamReader& operator=(const CompressedStreamReader&)=delete;

	// Instance Constructor
	//
	CompressedStreamReader(const std::unique_ptr<File>& file, size_t offset, size_t length);

	//-------------------------------------------------------------------------

	// CheckMagic
	//
	// Variadic function that recursively verifies a magic number
	static bool CheckMagic(void*, size_t) { return true; }

	template <typename _first, typename... _remaining>
	static bool CheckMagic(void* pointer, size_t length, _first first, _remaining... remaining)
	{
		// Ensure that the pointer is valid and there is at least one byte left
		if((pointer == nullptr) || (length == 0)) return false;

		// Cast the pointer into a uint8_t pointer and test the next byte
		_first* ptr = reinterpret_cast<_first*>(pointer);
		if(*ptr != first) return false;

		// Adjust the pointer and the length and recusrively continue to check magic numbers
		return CheckMagic(reinterpret_cast<uint8_t*>(ptr) + sizeof(_first), length - sizeof(_first), remaining...);
	}

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<MappedFileView>	m_view;		// Underlying mapped file view
	std::unique_ptr<StreamReader>	m_stream;	// Underlying stream implementation
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __COMPRESSEDSTREAMREADER_H_
