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

#include "stdafx.h"					// Include project pre-compiled headers
#include "FsObject.h"				// Include FsObject declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// FsObject Constructor
//
// Arguments:
//
//	NONE

FsObject::FsObject()
{
	// Initialize the fsobject_t structure to all zeros
	memset(static_cast<fsobject_t*>(this), 0, sizeof(fsobject_t));
}

//-----------------------------------------------------------------------------
// FsObject Constructor
//
// Arguments:
//
//	rhs			- FsObject instance to copy

FsObject::FsObject(const fsobject_t& rhs)
{
	// Copy the right-hand structure
	memcpy(static_cast<fsobject_t*>(this), &rhs, sizeof(fsobject_t));

	// If the right-hand has allocated physical.ospath, allocate a unique
	// buffer for this instance and copy the string over
	if(rhs.physical.ospath) {

		size_t cch = wcslen(rhs.physical.ospath) + 1;
		physical.ospath = reinterpret_cast<wcharptr_t>(midl_user_allocate(cch * sizeof(wchar_t)));
		wcscpy_s(physical.ospath, cch, rhs.physical.ospath);
	}
}

//-----------------------------------------------------------------------------
// FsObject Destructor

FsObject::~FsObject()
{
	// Release embedded RPC data pointers within the fsobject_t structure
	if(fsobject_t::physical.ospath) midl_user_free(fsobject_t::physical.ospath);
}

//-----------------------------------------------------------------------------
// FsObject::operator=
//
// Arguments:
//
//	rhs			- Right-hand side object to assign
FsObject& FsObject::operator=(const fsobject_t& rhs)
{
	// Release any embedded string pointers before copying the other structure
	if(physical.ospath) midl_user_free(physical.ospath);

	// Copy the right-hand structure
	memcpy(static_cast<fsobject_t*>(this), &rhs, sizeof(fsobject_t));

	// If the right-hand has allocated physical.ospath, allocate a unique
	// buffer for this instance and copy the string over
	if(rhs.physical.ospath) {

		size_t cch = wcslen(rhs.physical.ospath) + 1;
		physical.ospath = reinterpret_cast<wcharptr_t>(midl_user_allocate(cch * sizeof(wchar_t)));
		wcscpy_s(physical.ospath, cch, rhs.physical.ospath);
	}

	return *this;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)