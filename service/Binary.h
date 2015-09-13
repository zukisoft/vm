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

#ifndef __BINARY_H_
#define __BINARY_H_
#pragma once

#include <functional>
#include <memory>

#pragma warning(push, 4)

// Forward Declarations
//
class Executable;
class Host;

//-----------------------------------------------------------------------------
// Class Binary
//
// Interface that must be implemented by a binary image loader class, exposes
// metadata about the image after its loaded so that a Process instance can be
// constructed and initialized around it

struct __declspec(novtable) Binary
{
	// Binary::LoadFunction
	//
	// Function signature for a binary Load() implementation, which must be a public static method
	using LoadFunction = std::function<std::unique_ptr<Binary>(Host const* host, Executable const* executable)>;

	//-------------------------------------------------------------------------
	// Properties

	// BaseAddress
	//
	// Gets the base address of the loaded image
	__declspec(property(get=getBaseAddress)) void const* BaseAddress;
	virtual void const* getBaseAddress(void) const = 0;

	// EntryPoint
	//
	// Gets the entry point of the loaded image
	__declspec(property(get=getEntryPoint)) void const* EntryPoint;
	virtual void const* getEntryPoint(void) const = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __BINARY_H_
