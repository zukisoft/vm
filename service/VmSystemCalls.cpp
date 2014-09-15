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
#include "VmSystemCalls.h"

#pragma warning(push, 4)

VmSystemCalls::object_map_t VmSystemCalls::s_objectmap;
Concurrency::reader_writer_lock VmSystemCalls::s_objmaplock;

void VmSystemCalls::AddSystemCallsObject(const UUID& uuid, VmSystemCalls* instance)
{
	Concurrency::reader_writer_lock::scoped_lock lock(s_objmaplock);
	s_objectmap.insert(std::make_pair(uuid, instance));
}

void VmSystemCalls::RemoveSystemCallsObject(const UUID& uuid)
{
	Concurrency::reader_writer_lock::scoped_lock lock(s_objmaplock);
	s_objectmap.erase(uuid);
}

VmSystemCalls* VmSystemCalls::GetSystemCallsObject(const UUID& uuid)
{
	Concurrency::reader_writer_lock::scoped_lock_read lock(s_objmaplock);
	auto iterator = s_objectmap.find(uuid);
	if(iterator == s_objectmap.end()) return nullptr;
	else return iterator->second;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
