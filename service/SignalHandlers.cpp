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

#include "stdafx.h"
#include "SignalHandlers.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SignalHandlers::Create (static)
//
// Creates a new default signal handler collection
//
// Arguments:
//
//	NONE

std::shared_ptr<SignalHandlers> SignalHandlers::Create(void)
{
	// Create the SignalHandlers instance with a default collection
	return std::make_shared<SignalHandlers>();
}

//-----------------------------------------------------------------------------
// ProcessHandles::Duplicate (static)
//
// Duplicates an existing handle collection
//
// Arguments:
//
//	existing		- The existing collection instance to be duplicated

std::shared_ptr<SignalHandlers> SignalHandlers::Duplicate(const std::shared_ptr<SignalHandlers>& existing)
{
	// Create the SignalHandlers instance with the duplicated collection
	//return std::make_shared<ProcessHandles>(std::move(handles), std::move(fdpool));
	(existing);
	return std::make_shared<SignalHandlers>();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
