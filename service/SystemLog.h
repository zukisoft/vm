//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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

#ifndef __SYSTEMLOG_H_
#define __SYSTEMLOG_H_
#pragma once

#include <atomic>
#include "MemoryRegion.h"

#pragma warning(push, 4)
#pragma warning(disable:4200)		// zero-sized array in struct/union

//-----------------------------------------------------------------------------
// SystemLog
//
// Provides the system log functionality for a virtual machine, similar to the
// linux kernel ring buffer
//
// Refactor notes:
//
//  This will need to implement CharacterDevice later on, that's where the alternate
//  log output format comes into play.  There is /dev/kmsg and /proc/kmsg, so actually
//  it will probably be two wrapper classes.  THIS class needs to implement the syslog(2)
//  system call implementation
//
// the linux stuff is mostly in /fs/proc/kmsg.c and /kernel/printk/printk.c.  The
// internal log format does NOT need to be duplicated, the header portions are private
// and are never exposed to userspace
//
// Needs a polling implementation, perhaps just a condition variable that unblocks
// Read() when it triggers, but this likely needs to be cancellable -- review linux poll
//
// Needs a way to attach a console device, and control over the level that will be used
// to determine if the entry should be emitted or not

class SystemLog
{
public:

	// Instance Constructor
	//
	SystemLog(size_t size);

	// Destructor
	//
	~SystemLog()=default;

	// Level
	//
	// Strongly typed enumeration defining the level of a log entry
	enum class Level : int8_t
	{
		Default			= LINUX_LOGLEVEL_DEFAULT,		// Default (or last) log level
		Emergency		= LINUX_LOGLEVEL_EMERG,			// System is unusable
		Alert			= LINUX_LOGLEVEL_ALERT,			// Action must be taken immediately
		Critical		= LINUX_LOGLEVEL_CRIT,			// Critical conditions
		Error			= LINUX_LOGLEVEL_ERR,			// Error conditions
		Warning			= LINUX_LOGLEVEL_WARNING,		// Warning conditions
		Notice			= LINUX_LOGLEVEL_NOTICE,		// Normal but significant condition
		Informational	= LINUX_LOGLEVEL_INFO,			// Informational
		Debug			= LINUX_LOGLEVEL_DEBUG,			// Debug-level messages
	};

	//-------------------------------------------------------------------------
	// Member Functions

	//// Peek
	////
	//// Reads entries from the system log, does not clear them
	//size_t Peek(void* buffer, size_t length) { return Peek(SystemLogFormat::Standard, buffer, length); }
	//size_t Peek(SystemLogFormat format, void* buffer, size_t length);

	//// Pop
	////
	//// Reads entries from the system log and removes them
	//size_t Pop(void* buffer, size_t length) { return Pop(SystemLogFormat::Standard, buffer, length); }
	//size_t Pop(SystemLogFormat format, void* buffer, size_t length);

	// SetDefaultLevel
	//
	// Changes the default logging level
	void SetDefaultLevel(enum class Level level);

	// Write
	//
	// Writes an entry into the system log
	void Write(uint8_t facility, enum class Level level, char_t const* message, size_t length);

	//-------------------------------------------------------------------------
	// Properties

	//// Available
	////
	//// Gets the number of bytes available to be read from the log
	//__declspec(property(get=getAvailable)) size_t Available;
	//size_t getAvailable(void) { return Peek(nullptr, 0); }

	//// TODO: Need more properties, like the default console log level -- see syslog(2)

	// DefaultLevel
	//
	// Gets the default message logging level (non-console)
	__declspec(property(get=getDefaultLevel)) enum class Level DefaultLevel;
	enum class Level getDefaultLevel(void) const;

	//// Length
	////
	//// The adjusted length of the system log circular buffer
	//__declspec(property(get=getLength)) size_t Length;
	//size_t getLength(void) const { return m_buffer->Length; }

	//// TimestampBias
	////
	//// Gets/sets the bias to use when printing timestamp values; this value
	//// should come from a call to QueryPerformanceCounter() at startup
	//__declspec(property(get=getTimestampBias, put=putTimestampBias)) int64_t TimestampBias;
	//int64_t getTimestampBias(void) const { return m_tsbias; }
	//void putTimestampBias(int64_t value) { m_tsbias = value; }

private:

	SystemLog(SystemLog const&)=delete;
	SystemLog& operator=(SystemLog const&)=delete;

	// entry_t
	//
	// Represents the data contained in a single log entry
	struct entry_t
	{
		int64_t			timestamp;			// Entry timestamp
		uint16_t		entrylength;		// Overall entry length
		uint16_t		messagelength;		// Length of the message text
		uint8_t			facility : 5;		// Facility code
		uint8_t			level : 3;			// Entry level code
		uint8_t			reserved[3];		// Alignment padding (repurpose me)
		char_t			message[];			// Log message text

		// message[] is followed by padding to properly align the next entry
	};

	// MAX_BUFFER
	//
	// Controls the upper boundary on the system log ring buffer size
	static size_t const MAX_BUFFER	= (1 << 23);

	// MAX_MESSAGE
	//
	// Controls the upper boundary on the size of a single log message
	static size_t const MAX_MESSAGE = (UINT16_MAX - sizeof(entry_t));

	//-------------------------------------------------------------------------
	// Private Member Functions

	// GetTimestampBias (static)
	//
	// Gets the current system time to use as the timestamp bias
	static int64_t GetTimestampBias(void);

	// GetTimestampFrequency (static)
	//
	// Gets the performance counter timestamp frequency
	static double GetTimestampFrequency(void);

	// IncrementTailPointer
	//
	// Increments a tail pointer to reference the next log entry
	bool IncrementTailPointer(sync::reader_writer_lock::scoped_lock& lock, uintptr_t& tailptr);

	//// Print
	////
	//// Prints data from the log into an output buffer using a specified tail pointer
	//size_t Print(SystemLogFormat format, char* buffer, size_t length, uintptr_t& tailptr);

	//// PrintDeviceFormat
	////
	//// Formats a log entry to an output buffer in DEVICE format
	//size_t PrintDeviceFormat(LogEntry const* entry, char* buffer, size_t length);

	//// PrintStandardFormat
	////
	//// Formats a log entry to an output buffer in STANDARD format
	//size_t PrintStandardFormat(LogEntry const* entry, char* buffer, size_t length);

	//-------------------------------------------------------------------------
	// Member Variables

	double const						m_tsfreq;		// Timestamp frequency
	int64_t	const						m_tsbias;		// Timestamp bias
	std::unique_ptr<MemoryRegion>		m_buffer;		// Underlying buffer
	uintptr_t							m_top;			// Top of the buffer
	uintptr_t							m_bottom;		// Bottom of the buffer
	uintptr_t							m_head;			// Write position
	uintptr_t							m_tail;			// Read position
	std::atomic<enum class Level>		m_defaultlevel;	// Default message level
	mutable sync::reader_writer_lock	m_lock;			// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSTEMLOG_H_
