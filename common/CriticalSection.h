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

#ifndef __CRITICALSECTION_H_
#define __CRITICALSECTION_H_
#pragma once

#pragma warning(push, 4)			// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// CriticalSection
//
// Win32 CRITICAL_SECTION wrapper class

class CriticalSection
{
public:

	// Instance Constructors
	//
	CriticalSection() { InitializeCriticalSection(&m_cs); }
	CriticalSection(uint32_t spincount) { InitializeCriticalSectionAndSpinCount(&m_cs, spincount); }
	CriticalSection(uint32_t spincount, uint32_t flags) { InitializeCriticalSectionEx(&m_cs, spincount, flags); } 

	// Destructor
	//
	~CriticalSection() { DeleteCriticalSection(&m_cs); }

	//-----------------------------------------------------------------------
	// Member Functions

	// Enter
	//
	// Enters the critical section
	void Enter(void) { EnterCriticalSection(&m_cs); }

	// Leave
	//
	// Leaves the critical section
	void Leave(void) { LeaveCriticalSection(&m_cs); }

	// SetSpinCount
	//
	// Sets the spin count for the critical section object
	uint32_t SetSpinCount(uint32_t spincount) { return SetCriticalSectionSpinCount(&m_cs, spincount); } 

	// TryEnter
	//
	// Attempts to enter the critical section, fails if already entered
	bool TryEnter(void) { return (TryEnterCriticalSection(&m_cs) == TRUE); }

private:

	CriticalSection(const CriticalSection& rhs);
	CriticalSection& operator=(const CriticalSection& rhs);
	
	//-----------------------------------------------------------------------
	// Member Variables

	CRITICAL_SECTION		m_cs;		// Underlying CRITICAL_SECTION
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif		// __CRITICALSECTION_H_
