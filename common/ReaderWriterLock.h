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

#ifndef __READERWRITERLOCK_H_
#define __READERWRITERLOCK_H_
#pragma once

#pragma warning(push, 4)			

//---------------------------------------------------------------------------
// ReaderWriterLock
//
// Win32 Slim Reader/Writer lock wrapper class

class ReaderWriterLock
{
public:

	// Instance Constructor
	//
	ReaderWriterLock() { InitializeSRWLock(&m_lock); }

	//-----------------------------------------------------------------------
	// Member Functions

	// AcquireReader
	//
	// Acquires a shared reader lock
	void AcquireReader(void) { AcquireSRWLockShared(&m_lock); }

	// AcquireWriter
	//
	// Acquires an exclusive writer lock
	void AcquireWriter(void) { AcquireSRWLockExclusive(&m_lock); }

	// ReleaseReader
	//
	// Releases a shared reader lock
	void ReleaseReader(void) { ReleaseSRWLockShared(&m_lock); }

	// ReleaseWriter
	//
	// Releases an exclusive writer lock
	void ReleaseWriter(void) { ReleaseSRWLockExclusive(&m_lock); }

	// TryAcquireReader
	//
	// Attempts to acquire a shared reader lock
	bool TryAcquireReader(void) { return (TryAcquireSRWLockShared(&m_lock)) ? true : false; }

	// TryAcquireWriter
	//
	// Attempts to acquire an exclusive writer lock
	bool TryAcquireWriter(void) { return (TryAcquireSRWLockExclusive(&m_lock)) ? true : false; }

private:

	ReaderWriterLock(const ReaderWriterLock&)=delete;
	ReaderWriterLock& operator=(const ReaderWriterLock&)=delete;
	
	//-----------------------------------------------------------------------
	// Member Variables

	SRWLOCK					m_lock;			// Underlying SRWLOCK object
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif		// __READERWRITERLOCK_H_
