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

#ifndef __RANDOM_H_
#define __RANDOM_H_
#pragma once

#include "Exception.h"
#include "Win32Exception.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// Random (static)
//
// Random data generator

class Random
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Generate
	//
	// Generates random data into a destination buffer
	static void Generate(void* buffer, size_t length);

	// Generate<type>
	//
	// Generates a random value for a specific data type
	template<typename _type>
	static _type Generate(void) { _type value; Generate(&value, sizeof(_type)); return value; }

private:

	Random()=delete;
	~Random()=delete;
	Random(const Random&)=delete;
	Random& operator=(const Random&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	static HCRYPTPROV		s_provider;			// Crytography provider
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RANDOM_H_
