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

#ifndef __HANDLE_H_
#define __HANDLE_H_
#pragma once

#include <memory>
#include <linux/stat.h>
#include "NodeType.h"

#pragma warning(push, 4)

// Forward Declarations
//
struct __declspec(novtable) Alias;
struct __declspec(novtable) Node;

//-----------------------------------------------------------------------------
// Handle
//
// Interface that must be implemented for a file system handle instance

struct __declspec(novtable) Handle
{
	// Duplicate
	//
	// Creates a duplicate Handle instance
	virtual std::shared_ptr<Handle> Duplicate(int flags) = 0;

	// Read
	//
	// Synchronously reads data from the underlying node into a buffer
	virtual uapi::size_t Read(void* buffer, uapi::size_t count) = 0;

	// ReadAt
	//
	// Synchronously reads data from the underlying node into a buffer
	virtual uapi::size_t ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count) = 0;

	// Seek
	//
	// Changes the file position
	virtual uapi::loff_t Seek(uapi::loff_t offset, int whence) = 0;

	// Sync
	//
	// Synchronizes all metadata and data associated with the file to storage
	virtual void Sync(void) = 0;

	// SyncData
	//
	// Synchronizes all data associated with the file to storage, not metadata
	virtual void SyncData(void) = 0;

	// Write
	//
	// Synchronously writes data from a buffer to the underlying node
	virtual uapi::size_t Write(const void* buffer, uapi::size_t count) = 0;

	// WriteAt
	//
	// Synchronously writes data from a buffer to the underlying node
	virtual uapi::size_t WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count) = 0;

	// Alias
	//
	// Gets a reference to the Alias used to open this handle
	__declspec(property(get=getAlias)) std::shared_ptr<::Alias> Alias;
	virtual std::shared_ptr<::Alias> getAlias(void) = 0;

	// CloseOnExec
	//
	// Gets/sets the flag to close this handle during an execute operation
	__declspec(property(get=getCloseOnExec, put=putCloseOnExec)) bool CloseOnExec;
	virtual bool getCloseOnExec(void) = 0;
	virtual void putCloseOnExec(bool value) = 0;

	// Flags
	//
	// Gets a copy of the current handle flags
	__declspec(property(get=getFlags)) int Flags;
	virtual int getFlags(void) = 0;

	// Node
	//
	// Gets the node instance to which this alias references
	__declspec(property(get=getNode)) std::shared_ptr<::Node> Node;
	virtual std::shared_ptr<::Node> getNode(void) = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HANDLE_H_