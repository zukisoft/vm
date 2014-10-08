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

#include <functional>

#pragma warning(push, 4)

// Will need to maintain both the original program break passed in by the 
// service as well as the current program break globally ...
static void* g_baselinebreak = reinterpret_cast<void*>(0x08000000);
static void* g_break = reinterpret_cast<void*>(0x08000000);

//// would like to come up with a template of some kind to define the system 
//// call such that the arguments and return code can be strongly typed
////
//// PTR -> func(PCONTEXT) -> sys_func(int, long, void*, etc)
//
//template <typename _retval = void, typename _arg0 = void, typename _arg1 = void>
//class test
//{
//public:
//	
//	// this is the generic function
//	uapi::long_t operator()(PCONTEXT& context) 
//	{
//		// will need placeholders? how to handle arguments, perhaps with enable_if<>??
//		// static_cast or reinterpret_cast depends on if result is pointer type
//		return reinterpret_cast<uapi::long_t>(*this(reinterpret_cast<void*>(context->Ebx)));
//	}
//
//	// enable_if 1 argument, 2 arguments, 3 arguments, 4 arguments?
//
//	void* operator()(void* address)
//	{
//		return nullptr;
//	}
//};

void* sys_brk(void* address)
{
	MEMORY_BASIC_INFORMATION          meminfo;						

	if(address == nullptr) return g_break;

	intptr_t current = intptr_t(g_break);
	intptr_t delta = align::up(intptr_t(address) - current, 0x10000);      // <-- MemoryRegion::AllocationGranularity

	// INCREASE PROGRAM BREAK
	if(delta > 0) {

		VirtualQuery(reinterpret_cast<void*>(current), &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
		if(meminfo.State == MEM_FREE) {
                     
			// Only ask for as much as is                    available to reserve
			delta = min(delta, align::down(meminfo.RegionSize, 0x10000));   // <-- MemoryRegion::AllocationGranularity

			// Attempt to reserve and commit the calcuated region with READWRITE access
			void* result = VirtualAlloc(reinterpret_cast<void*>(current), delta, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(result == nullptr) { /* TODO: WIN32EXCEPTION */ }

			g_break = reinterpret_cast<void*>(current + delta);
		}
	}

	// DECREASE PROGRAM BREAK
	else if(delta < 0) {

		// Determine the target address, which can never be below the original program break
		intptr_t target = max(intptr_t(g_baselinebreak), (current + delta));

		// Work backwards from the previously allocated region to release as many as possible
		VirtualQuery(reinterpret_cast<void*>(current - 1), &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
		while(target <= intptr_t(meminfo.AllocationBase)) {

			// Release the region
			if(!VirtualFree(meminfo.AllocationBase, 0, MEM_RELEASE)) { /* TODO: WIN32EXCEPTION */ }

			// Align the current break pointer with the released region base address and get
			// information about the allocation immediately prior to keep going
			current = intptr_t(meminfo.AllocationBase);
			VirtualQuery(reinterpret_cast<void*>(current - 1), &meminfo, sizeof(MEMORY_BASIC_INFORMATION));
		}

		g_break = reinterpret_cast<void*>(current);
	}

	return g_break;                   // Return the (possibly) updated program break pointer
}

//-----------------------------------------------------------------------------
// void* brk(void* address)
//
// EBX	- void*		address
// ECX
// EDX
// ESI
// EDI
// EBP
//
uapi::long_t sys045_brk(PCONTEXT context)
{
	_ASSERTE(context->Eax == 45);			// Verify system call number
	return 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


