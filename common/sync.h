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

#ifndef __SYNC_H_
#define __SYNC_H_
#pragma once

#include <stdint.h>
#include <Windows.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Synchronization Primitives
//
// These are generally modeled after their counterparts in the Microsoft Parallel
// Patterns Library "Concurrency" namespace, but those classes do not perform 
// well in general use outside of that implementation.  In some cases, like with
// the reader_writer_lock, the primitive Win32 implementation can be orders of
// magnitude faster.
//-----------------------------------------------------------------------------

namespace sync {

// critical_section
//
// Win32 critical section implementation
class critical_section
{
public:

	// Instance Constructor
	//
	critical_section() { InitializeCriticalSection(&m_cs); }

	// Destructor
	//
	~critical_section() { DeleteCriticalSection(&m_cs); }

	//-----------------------------------------------------------------------
	// Member Functions

	// lock
	//
	// Enters the critical section
	void lock(void) { EnterCriticalSection(&m_cs); }

	// try_lock
	//
	// Attempts to enter the critical section, fails if already entered
	bool try_lock(void) { return (TryEnterCriticalSection(&m_cs) == TRUE); }

	// unlock
	//
	// Leaves the critical section
	void unlock(void) { LeaveCriticalSection(&m_cs); }

	// critical_section::scoped_lock
	//
	class scoped_lock
	{
	public:

		// Instance Constructor
		explicit scoped_lock(critical_section& cs) : m_cs(cs), m_held(true) { m_cs.lock(); }

		// Destructor
		~scoped_lock() { if(m_held) m_cs.unlock(); }

		//---------------------------------------------------------------------
		// Member Functions

		// unlock
		//
		// Unlocks the scoped lock before it falls out of scope
		void unlock(void) { if(m_held) m_cs.unlock(); m_held = false; }

	private:

		scoped_lock(const scoped_lock&)=delete;
		scoped_lock& operator=(const scoped_lock&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		critical_section&		m_cs;		// Referenced critical_section
		bool					m_held;		// Flag if critical_section is held
	};

private:

	critical_section(const critical_section&)=delete;
	critical_section& operator=(const critical_section& rhs)=delete;
	
	//-----------------------------------------------------------------------
	// Member Variables

	CRITICAL_SECTION		m_cs;		// Underlying CRITICAL_SECTION
};

// reader_writer_lock
//
// Win32 slim reader/writer lock implementation
class reader_writer_lock
{
public:

	// Instance Constructor
	//
	reader_writer_lock() : m_srwl(SRWLOCK_INIT) {}

	// Destructor
	//
	~reader_writer_lock()=default;

	//-----------------------------------------------------------------------
	// Member Functions

	// lock_read
	//
	// Acquires a shared reader lock
	void lock_read(void) { AcquireSRWLockShared(&m_srwl); }

	// lock_write
	//
	// Acquires an exclusive writer lock
	void lock_write(void) { AcquireSRWLockExclusive(&m_srwl); }

	// try_lock_read
	//
	// Attempts to acquire a shared reader lock
	bool try_lock_read(void) { return (TryAcquireSRWLockShared(&m_srwl)) ? true : false; }

	// try_lock_write
	//
	// Attempts to acquire an exclusive writer lock
	bool try_lock_write(void) { return (TryAcquireSRWLockExclusive(&m_srwl)) ? true : false; }

	// unlock_read
	//
	// Releases a shared reader lock
	void unlock_read(void) { ReleaseSRWLockShared(&m_srwl); }

	// unlock_write
	//
	// Releases an exclusive writer lock
	void unlock_write(void) { ReleaseSRWLockExclusive(&m_srwl); }

	// reader_writer_lock::scoped_lock_read
	//
	class scoped_lock_read
	{
	public:

		// Instance Constructor
		//
		explicit scoped_lock_read(reader_writer_lock& rwl) : m_rwl(rwl), m_held(true) { m_rwl.lock_read(); }

		// Destructor
		//
		~scoped_lock_read() { if(m_held) m_rwl.unlock_read(); }

		//---------------------------------------------------------------------
		// Member Functions

		// unlock
		//
		// Unlocks the scoped lock before it falls out of scope
		void unlock(void) { if(m_held) m_rwl.unlock_read(); m_held = false; }

	private:

		scoped_lock_read(const scoped_lock_read&)=delete;
		scoped_lock_read& operator=(const scoped_lock_read&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		reader_writer_lock&			m_rwl;		// Referenced reader_writer_lock
		bool						m_held;		// Flag if the reader is held
	};

	// reader_writer_lock::scoped_lock_write
	//
	class scoped_lock_write
	{
	public:

		// Instance Constructor
		//
		explicit scoped_lock_write(reader_writer_lock& rwl) : m_rwl(rwl), m_held(true) { m_rwl.lock_write(); }

		// Destructor
		//
		~scoped_lock_write() { if(m_held) m_rwl.unlock_write(); }

		//---------------------------------------------------------------------
		// Member Functions

		// unlock
		//
		// Unlocks the scoped lock before it falls out of scope
		void unlock(void) { if(m_held) m_rwl.unlock_write(); m_held = false; }

	private:

		scoped_lock_write(const scoped_lock_write&)=delete;
		scoped_lock_write& operator=(const scoped_lock_write&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		reader_writer_lock&			m_rwl;		// Referenced reader_writer_lock
		bool						m_held;		// Flag if the writer is held
	};

private:

	reader_writer_lock(const reader_writer_lock&)=delete;
	reader_writer_lock& operator=(const reader_writer_lock&)=delete;
	
	//-----------------------------------------------------------------------
	// Member Variables

	SRWLOCK						m_srwl;			// Underlying SRWLOCK object
};

//---------------------------------------------------------------------------

}	// namespace sync

#pragma warning(pop)

#endif	// __SYNC_H_
