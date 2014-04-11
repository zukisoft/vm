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

#ifndef __FSOBJECT_H_
#define __FSOBJECT_H_

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// FsObject
//
// Wrapper class for fsobject_t to provide proper release of embedded pointers

class FsObject : public fsobject_t
{
public:

	// Instance Constructors
	//
	FsObject();
	FsObject(const FsObject& rhs) : FsObject(static_cast<fsobject_t>(rhs)) {}
	FsObject(const fsobject_t& object);

	// Destructor
	//
	~FsObject();

	//-------------------------------------------------------------------------
	// Overloaded Operators

	// fsobject_t&
	//
	operator fsobject_t&() { return *static_cast<fsobject_t*>(this); }

	// fsobject_t*
	//
	operator fsobject_t*() { return static_cast<fsobject_t*>(this); }

	// assignment
	//
	FsObject& operator=(const FsObject& rhs) { return operator=(static_cast<fsobject_t>(rhs)); }
	FsObject& operator=(const fsobject_t& rhs);

	// address-of
	fsobject_t* operator&() { return static_cast<fsobject_t*>(this); }
};


//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FSOBJECT_H_
