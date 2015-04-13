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
#include "Namespace.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Namespace Constructor (private)
//
// Arguments:
//
//	pids		- PidNamespace instance to contain

Namespace::Namespace(const std::shared_ptr<PidNamespace>& pids) : m_pids(pids)
{
}

//-----------------------------------------------------------------------------
// Namespace::Create
//
// Creates a new Namespace instance
//
// Arguments:
//
//	NONE

std::shared_ptr<Namespace> Namespace::Create(void)
{
	return std::make_shared<Namespace>(PidNamespace::Create());
}

//-----------------------------------------------------------------------------
// Namespace::getPids
//
// Gets a reference to the contained PID namespace

std::shared_ptr<PidNamespace> Namespace::getPids(void) const
{
	return m_pids;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
