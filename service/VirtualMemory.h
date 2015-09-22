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

#ifndef __VIRTUALMEMORY_H_
#define __VIRTUALMEMORY_H_
#pragma once

#include <stdint.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Interface VirtualMemory
//
// Provides an interface that defines operations required to allocate, release, 
// and manipulate virtual memory within a process
//
// TODO: Add a type flag to Reserve() [heap, stack, image, etc] and methods/properties 
// for tracking and reporting on memory utilization

struct __declspec(novtable) VirtualMemory
{
	// VirtualMemory::Protection
	//
	// Generalized protection flags used with memory operations
	struct Protection final : public bitmask<Protection, uint8_t, 0x01 /* Execute */ | 0x02 /* Read */ | 0x04 /* Write */ | 0x80 /* Guard */>
	{
		using bitmask::bitmask;

		//-------------------------------------------------------------------------
		// Fields

		// Execute (static)
		//
		// Indicates that the memory region can be executed
		static Protection const Execute;

		// Guard (static)
		//
		// Indicates that the memory region consists of guard pages
		static Protection const Guard;

		// None (static)
		//
		// Indicates that the memory region cannot be accessed
		static Protection const None;

		// Read (static)
		//
		// Indicates that the memory region can be read
		static Protection const Read;

		// Write (static)
		//
		// Indicates that the memory region can be written to
		static Protection const Write;
	};

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates a virtual memory region
	virtual uintptr_t Allocate(size_t length, VirtualMemory::Protection protection) = 0;
	virtual uintptr_t Allocate(uintptr_t address, size_t length, VirtualMemory::Protection protection) = 0;

	// Lock
	//
	// Attempts to lock a region into physical memory
	virtual void Lock(uintptr_t address, size_t length) const = 0;

	// Map
	//
	// Maps a virtual memory region into the calling process
	virtual void* Map(uintptr_t address, size_t length, VirtualMemory::Protection protection) = 0;

	// Protect
	//
	// Sets the memory protection flags for a virtual memory region
	virtual void Protect(uintptr_t address, size_t length, VirtualMemory::Protection protection) const = 0;

	// Read
	//
	// Reads data from a virtual memory region into the calling process
	virtual size_t Read(uintptr_t address, void* buffer, size_t length) const = 0;

	// Release
	//
	// Releases a virtual memory region
	virtual void Release(uintptr_t address, size_t length) = 0;

	// Reserve
	//
	// Reserves a virtual memory region for later allocation
	virtual uintptr_t Reserve(size_t length) = 0;
	virtual uintptr_t Reserve(uintptr_t address, size_t length) = 0;

	// Unlock
	//
	// Attempts to unlock a region from physical memory
	virtual void Unlock(uintptr_t address, size_t length) const = 0;

	// Unmap
	//
	// Unmaps a previously mapped memory region from the calling process
	virtual void Unmap(void const* mapping) = 0;

	// Write
	//
	// Writes data into a virtual memory region from the calling process
	virtual size_t Write(uintptr_t address, void const* buffer, size_t length) const = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALMEMORY_H_
