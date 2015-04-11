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

#ifndef __NAMESPACE_H_
#define __NAMESPACE_H_
#pragma once

#include <memory>
#include <linux/types.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Namespace
//
// Implements a namespace, which defines a view of global resources (PIDs,
// UIDs/GIDs, mount points, etc) for a process

class Namespace
{
public:

	// Destructor
	//
	~Namespace()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	//-------------------------------------------------------------------------
	// Properties

private:

	Namespace(const Namespace&)=delete;
	Namespace& operator=(const Namespace&)=delete;

	// Instance Constructor
	//
	Namespace();
	friend class std::_Ref_count_obj<Namespace>;

	//-------------------------------------------------------------------------
	// Member Variables
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NAMESPACE_H_
