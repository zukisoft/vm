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

#include "stdafx.h"

#pragma warning(push, 4)

// g_break
//
// Current program break address
void* g_break = nullptr;

// g_startupinfo (main.cpp)
//
// Process startup information provided by the service
extern sys32_startup_info g_startupinfo;

// s_sysinfo
//
// Static copy of the system information; required to know allocation granularity
static SYSTEM_INFO s_sysinfo = []() -> SYSTEM_INFO {

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo;
}();

//-----------------------------------------------------------------------------
// sys_brk
//
// Sets the process program break, which is extra space reserved by a process
// to implement a heap.  Specify nullptr to get the current break address.  This
// function is not capable of returning an error code on Linux, to indicate that
// the operation could not be completed, return the previously set break address
//
// Arguments:
//
//	address		- Address to set the program break to

uapi::long_t sys_brk(void* address)
{
	MEMORY_BASIC_INFORMATION	meminfo;	// Information on a memory region

	// NULL can be passed in as the address to retrieve the current program break
	if(address == nullptr) return reinterpret_cast<uapi::long_t>(g_break);

	// Create a working copy of the current break and calcuate the requested delta
	intptr_t current = intptr_t(g_break);
	intptr_t delta = align::up(intptr_t(address) - current, s_sysinfo.dwAllocationGranularity);

	// INCREASE PROGRAM BREAK
	if(delta > 0) {

		// Check to see if the region at the currently set break is MEM_FREE
		VirtualQuery(reinterpret_cast<void*>(current), &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
		if(meminfo.State == MEM_FREE) {
                     
			// Only ask for as much as is available contiguously
			delta = min(delta, align::down(intptr_t(meminfo.RegionSize), s_sysinfo.dwAllocationGranularity));

			// Attempt to reserve and commit the calcuated region with READWRITE access
			void* result = VirtualAlloc(reinterpret_cast<void*>(current), delta, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(result) g_break = reinterpret_cast<void*>(current + delta);
		}
	}

	// DECREASE PROGRAM BREAK
	else if(delta < 0) {

		// Determine the target address, which can never be below the original program break
		intptr_t target = max(intptr_t(g_startupinfo.program_break), (current + delta));

		// Work backwards from the previously allocated region to release as many as possible
		VirtualQuery(reinterpret_cast<void*>(current - 1), &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
		while(target <= intptr_t(meminfo.AllocationBase)) {

			// Attempt to decommit and release the entire region
			if(!VirtualFree(meminfo.AllocationBase, 0, MEM_RELEASE)) break;

			// Align the current break pointer with the released region's base address
			current = intptr_t(meminfo.AllocationBase);

			// Get information on the next region lower in memory to check if it can be released
			VirtualQuery(reinterpret_cast<void*>(current - 1), &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
		}

		// Reset the program break to the last successfully released region base address
		g_break = reinterpret_cast<void*>(current);
	}

	///// TODO: TESTING - CLEAN THIS UP (now this works with glibc, but un-hack it)
	uintptr_t result = uintptr_t(address);
	if((result >= uintptr_t(g_startupinfo.program_break)) && (result <= uintptr_t(g_break)))
	{
		return reinterpret_cast<uapi::long_t>(address);
	}
	else return 0;
	/////////////////////////////

//	return reinterpret_cast<uapi::long_t>(g_break);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


