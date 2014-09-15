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

#ifndef __VMSYSTEMCALLS_H_
#define __VMSYSTEMCALLS_H_
#pragma once

#include <concrt.h>

#pragma warning(push, 4)			

//-----------------------------------------------------------------------------
// Interface SystemCalls
//
// Build-specific (32bit vs 64bit) implementation of the system calls interface;
// this will be invoked from the RPC entry points

class VmSystemCalls
{
public:


	static VmSystemCalls* GetSystemCallsObject(const UUID& uuid);

private:

	VmSystemCalls(const VmSystemCalls&)=delete;
	VmSystemCalls& operator=(const VmSystemCalls&)=delete;

	// uuid_key_comp
	//
	// Key comparison function for UUIDs when used as a collection key
	struct uuid_key_comp 
	{ 
		bool operator() (const UUID& lhs, const UUID& rhs) const 
		{
			// key_comp functions are comparable to a < operation
			return (memcmp(&lhs, &rhs, sizeof(UUID)) < 0);
		}
	};

	// object_map_t
	//
	// RPC object -> SystemCalls interface map type
	using object_map_t = std::map<UUID, VmSystemCalls*, uuid_key_comp>;

	static object_map_t s_objectmap;
	static Concurrency::reader_writer_lock s_objmaplock;
	static void AddSystemCallsObject(const UUID& uuid, VmSystemCalls* instance);
	static void RemoveSystemCallsObject(const UUID& uuid);};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMSYSTEMCALLS_H_
