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

#ifndef __ARCHITECTURE_TRAITS_H_
#define __ARCHITECTURE_TRAITS_H_
#pragma once

#include <stdint.h>
#include "Architecture.h"
#include "Exception.h"

#pragma warning(push, 4)

// architecture_traits
//
template <Architecture architecture> struct architecture_traits {};

// architecture_traits<x86>
//
template <> struct architecture_traits<Architecture::x86>
{
#ifndef _M_X64
	typedef CONTEXT	context_t;
	static void CheckPointer(void*) {}
#else
	typedef WOW64_CONTEXT			context_t;
	static void CheckPointer(void* pointer) { if(uintptr_t(pointer) > UINT32_MAX) throw Exception(E_ARCHITECTUREPOINTER); }
#endif

	architecture_traits(const architecture_traits&)=delete;
	architecture_traits& operator=(const architecture_traits&)=delete;
};

#ifdef _M_X64
// architecture_traits<x86_64>
//
template <> struct architecture_traits<Architecture::x86_64>
{
	typedef CONTEXT					context_t;
	static void CheckPointer(void*) {}

	static const size_t				maxaddress = UINT64_MAX;

	architecture_traits(const architecture_traits&)=delete;
	architecture_traits& operator=(const architecture_traits&)=delete;
};
#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ARCHITECTURE_TRAITS_H_
