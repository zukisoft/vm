//-----------------------------------------------------------------------------
// Copyright (c) 1999-2014 Michael G. Brehm
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

#ifndef __AUTOCRITICALSECTION_H_
#define __AUTOCRITICALSECTION_H_
#pragma once

#include "CriticalSection.h"

#pragma warning(push, 4)			

//-----------------------------------------------------------------------------
// AutoCriticalSection
//
// Provides a stack-based automatic enter/leave wrapper around a CriticalSection

class AutoCriticalSection
{
public:

	// Instance Constructor
	//
	explicit AutoCriticalSection(CriticalSection& cs) : m_cs(cs) { m_cs.Enter(); }

	// Desructor
	~AutoCriticalSection() { m_cs.Leave(); }

private:

	AutoCriticalSection(const AutoCriticalSection&)=delete;
	AutoCriticalSection& operator=(const AutoCriticalSection&)=delete;

	//-----------------------------------------------------------------------
	// Member Variables

	CriticalSection&			m_cs;		// Referenced CriticalSection
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif		// __AUTOCRITICALSECTION_H_
