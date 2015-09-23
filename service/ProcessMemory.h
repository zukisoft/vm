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

#ifndef __PROCESSMEMORY_H_
#define __PROCESSMEMORY_H_
#pragma once

#include <stdint.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Interface ProcessMemory
//
// Provides an interface that defines operations required to allocate, release, 
// and manipulate a process' virtual memory address space

struct __declspec(novtable) ProcessMemory
{
	// ProcessMemory::Protection
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

	// AllocateMemory
	//
	// Allocates a virtual memory region
	virtual uintptr_t AllocateMemory(size_t length, ProcessMemory::Protection protection) = 0;
	virtual uintptr_t AllocateMemory(uintptr_t address, size_t length, ProcessMemory::Protection protection) = 0;

	// LockMemory
	//
	// Attempts to lock a region into physical memory
	virtual void LockMemory(uintptr_t address, size_t length) const = 0;

	// MapMemory
	//
	// Maps a virtual memory region into the calling process
	virtual void* MapMemory(uintptr_t address, size_t length, ProcessMemory::Protection protection) = 0;

	// ProtectMemory
	//
	// Sets the memory protection flags for a virtual memory region
	virtual void ProtectMemory(uintptr_t address, size_t length, ProcessMemory::Protection protection) const = 0;

	// ReadMemory
	//
	// Reads data from a virtual memory region into the calling process
	virtual size_t ReadMemory(uintptr_t address, void* buffer, size_t length) const = 0;

	// ReleaseMemory
	//
	// Releases a virtual memory region
	virtual void ReleaseMemory(uintptr_t address, size_t length) = 0;

	// ReserveMemory
	//
	// Reserves a virtual memory region for later allocation
	virtual uintptr_t ReserveMemory(size_t length) = 0;
	virtual uintptr_t ReserveMemory(uintptr_t address, size_t length) = 0;

	// UnlockMemory
	//
	// Attempts to unlock a region from physical memory
	virtual void UnlockMemory(uintptr_t address, size_t length) const = 0;

	// UnmapMemory
	//
	// Unmaps a previously mapped memory region from the calling process
	virtual void UnmapMemory(void const* mapping) = 0;

	// WriteMemory
	//
	// Writes data into a virtual memory region from the calling process
	virtual size_t WriteMemory(uintptr_t address, void const* buffer, size_t length) const = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESSMEMORY_H_