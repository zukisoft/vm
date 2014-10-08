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

#ifndef __MEMORYREGION_H_
#define __MEMORYREGION_H_
#pragma once

#include <memory>
#include "align.h"
#include "Exception.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// MemoryRegion
//
// Wrapper class to contain a memory region allocated with VirtualAlloc(Ex) so
// that an automatic destructor can be provided to release it
//
// TODO: Not sure why I added all that alignment crap in the bulk of the member
// functions, it doesn't really need to be there?  Need to test everything, and
// if they still work just remove it all.  These functions aren't that hard to call?!?!

class MemoryRegion
{
private:

	// Forward Declarations
	//
	class ReservationInfo;

public:

	// Destructor
	//
	~MemoryRegion();

	//-------------------------------------------------------------------------
	// Member Functions

	// Commit
	//
	// Commits page(s) within the region
	void* Commit(void* address, size_t length, uint32_t protect);

	// Decommit
	//
	// Decommits page(s) within the region
	void* Decommit(void* address, size_t length);

	// Detach
	//
	// Detaches the memory region from the class instance
	void* Detach(void) { return Detach(nullptr); }
	void* Detach(size_t* length);

	// Lock
	//
	// Locks page(s) within the region into physical memory
	void* Lock(void* address, size_t length);

	// Protect
	//
	// Applies protection flags to page(s) within the region
	void* Protect(void* address, size_t length, uint32_t protect);

	// Reserve (static)
	//
	// Reserves a range of virtual memory; can also commit at the same time by specifying
	// MEM_COMMIT to an overload that accepts a flags argument
	static std::unique_ptr<MemoryRegion> Reserve(size_t length)
		{ return Reserve(INVALID_HANDLE_VALUE, length, nullptr, MEM_RESERVE, PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(size_t length, uint32_t flags)
		{ return Reserve(INVALID_HANDLE_VALUE, length, nullptr, MEM_RESERVE | flags, (flags & MEM_COMMIT) ? PAGE_READWRITE : PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(size_t length, void* address)
		{ return Reserve(INVALID_HANDLE_VALUE, length, address, MEM_RESERVE, PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(size_t length, void* address, uint32_t flags)
		{ return Reserve(INVALID_HANDLE_VALUE, length, address, MEM_RESERVE | flags, (flags & MEM_COMMIT) ? PAGE_READWRITE : PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(HANDLE process, size_t length)
		{ return Reserve(process, length, nullptr, MEM_RESERVE, PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(HANDLE process, size_t length, uint32_t flags)
		{ return Reserve(process, length, nullptr, MEM_RESERVE | flags, (flags & MEM_COMMIT) ? PAGE_READWRITE : PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(HANDLE process, size_t length, void* address)
		{ return Reserve(process, length, address, MEM_RESERVE, PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(HANDLE process, size_t length, void* address, uint32_t flags)
		{ return Reserve(process, length, address, MEM_RESERVE | flags, (flags & MEM_COMMIT) ? PAGE_READWRITE : PAGE_NOACCESS); }

	// Unlock
	//
	// Unlocks page(s) within the region so that they can be swapped out
	void* Unlock(void* address, size_t length);

	//-------------------------------------------------------------------------
	// Fields

	// AllocationGranularity
	//
	// Exposes the system allocation granularity
	static const size_t AllocationGranularity;

	// PageSize
	//
	// Exposes the system page size
	static const size_t PageSize;

	//-------------------------------------------------------------------------
	// Properties

	// Length
	//
	// Gets the length of the memory region
	__declspec(property(get=getLength)) size_t Length;
	size_t getLength(void) const { return m_length; }

	// Pointer
	//
	// Gets the base pointer for the memory region
	__declspec(property(get=getPointer)) void* Pointer;
	void* getPointer(void) const { return m_base; }

	// Reservation
	//
	// Gets a reference to the contained ReservationInfo class
	__declspec(property(get=getReservation)) const ReservationInfo& Reservation;
	const ReservationInfo& getReservation(void) const { return m_reservation; }

private:

	MemoryRegion(const MemoryRegion&)=delete;
	MemoryRegion& operator=(const MemoryRegion&)=delete;

	// Instance Constructor
	//
	MemoryRegion(HANDLE process, void* base, size_t length, MEMORY_BASIC_INFORMATION&& meminfo) : m_process(process), m_base(base), m_length(length), m_reservation(std::move(meminfo)) {}
	friend std::unique_ptr<MemoryRegion> std::make_unique<MemoryRegion, HANDLE&, void*&, size_t&, MEMORY_BASIC_INFORMATION>(HANDLE&, void*&, size_t&, MEMORY_BASIC_INFORMATION&&);

	// ReservationInfo
	//
	// Stores the underlying allocation information for the memory region
	class ReservationInfo
	{
	public:

		ReservationInfo(MEMORY_BASIC_INFORMATION&& meminfo) : m_base(meminfo.AllocationBase), 
			m_length(align::up(meminfo.RegionSize, MemoryRegion::AllocationGranularity)) {}
		~ReservationInfo()=default;

		//---------------------------------------------------------------------
		// Properties

		// BaseAddress
		//
		// Gets the base address of the memory reservation
		__declspec(property(get=getBaseAddress)) const void* BaseAddress;
		const void* getBaseAddress(void) const { return m_base; }

		// Length
		//
		// Gets the lenght of the memory reservation
		__declspec(property(get=getLength)) size_t Length;
		size_t getLength(void) const { return m_length; }

	private:

		ReservationInfo(const ReservationInfo&)=delete;
		ReservationInfo& operator=(const ReservationInfo&)=delete;

		//---------------------------------------------------------------------
		// Member Variables
		const void*		m_base;				// Reservation base address
		size_t			m_length;			// Reservation length
	};

	// SystemInfo
	//
	// Used to initialize a static SYSTEM_INFO structure
	struct SystemInfo : public SYSTEM_INFO
	{
		SystemInfo() { GetNativeSystemInfo(static_cast<SYSTEM_INFO*>(this)); }
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	// Reserve
	//
	// Reserves a range of virtual memory
	static std::unique_ptr<MemoryRegion> Reserve(HANDLE process, size_t length, void* address, uint32_t flags, uint32_t protect);

	//-------------------------------------------------------------------------
	// Member Variables

	void*					m_base;			// Base pointer for the memory region
	size_t					m_length;		// Length of the memory region
	HANDLE					m_process;		// Process to operate against
	const ReservationInfo	m_reservation;	// Reservation information
	static SystemInfo		s_sysinfo;		// System information class
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MEMORYREGION_H_
