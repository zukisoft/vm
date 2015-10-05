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
#include "SystemLog.h"

#include "Capability.h"
#include "SystemInformation.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SystemLog Constructor
//
// Arguments:
//
//	size		- Size of the system log ring buffer in bytes

SystemLog::SystemLog(size_t size) : m_tsfreq(GetTimestampFrequency()), m_tsbias(GetTimestampBias()), m_defaultlevel(Level::Warning)
{
	// Minimum log size is the page size, maximum is constant MAX_BUFFER
	size = std::min(std::max(size, SystemInformation::PageSize), MAX_BUFFER);

	// Attempt to allocate the log buffer from virtual memory, don't use the heap
	m_buffer = MemoryRegion::Reserve(size, MEM_COMMIT);

	// Initialize the top, head and tail pointers to the top of the buffer
	m_top = m_head = m_tail = uintptr_t(m_buffer->Pointer);

	// Knowing the bottom of the buffer without calculating it is useful
	m_bottom = m_top + m_buffer->Length;
}

//-----------------------------------------------------------------------------
// SystemLog::getDefaultLevel
//
// Gets the default message logging level

enum class SystemLog::Level SystemLog::getDefaultLevel(void) const
{
	return m_defaultlevel;
}

//-----------------------------------------------------------------------------
// SystemLog::GetTimestampBias (private, static)
//
// Gets the current time to use as the timestamp bias
//
// Arguments:
//
//	NONE

int64_t SystemLog::GetTimestampBias(void)
{
	LARGE_INTEGER				qpcbias;		// QueryPerformanceCounter bias

	if(!QueryPerformanceCounter(&qpcbias)) throw Win32Exception{ GetLastError() };
	return qpcbias.QuadPart;
}

//-----------------------------------------------------------------------------
// SystemLog::GetTimestampFrequency (private, static)
//
// Gets the frequency of the high performance timer object
//
// Arguments:
//
//	NONE

double SystemLog::GetTimestampFrequency(void)
{
	LARGE_INTEGER				qpcfreq;		// QueryPerformanceCounter frequency

	if(!QueryPerformanceFrequency(&qpcfreq)) throw Win32Exception{ GetLastError() };
	return static_cast<double>(qpcfreq.QuadPart);
}

//-----------------------------------------------------------------------------
// SystemLog::IncrementTailPointer (private)
//
// Increments a tail pointer to point at the next system log entry
//
// Arguments:
//
//	lock		- Reference to a scoped_lock (unused, ensures caller has one)
//	tailptr		- Tail pointer to be incremented

inline bool SystemLog::IncrementTailPointer(sync::reader_writer_lock::scoped_lock& lock, uintptr_t& tailptr)
{
	UNREFERENCED_PARAMETER(lock);		// This is just to ensure the caller has locked

	// If the tail is in the same position as the head, the buffer is empty
	if(tailptr == m_head) return false;

	// Get the length of the entry currently being pointed to
	uint16_t entrylength = reinterpret_cast<entry_t*>(tailptr)->entrylength;

	// Increment the tail to point at the next entry in the buffer  
	tailptr += entrylength;

	// If there isn't enough room left to hold another entry header, or the length 
	// has been set to 0xFFFF, move the tail pointer back to the top of the buffer
	if((tailptr + sizeof(entry_t) > m_bottom) || (entrylength == UINT16_MAX)) tailptr = m_top;

	return true;
}

//-----------------------------------------------------------------------------
// SystemLog::Peek
//
// Reads entries from the system log without removing them
//
// Arguments:
//
//	format		- Output format
//	buffer		- Output buffer
//	len			- Output buffer size, in bytes

//size_t SystemLog::Peek(SystemLogFormat format, void* buffer, size_t length)
//{
//	// Peek is non-destructive, clone the member tail pointer
//	uintptr_t tailptr = m_tail;
//	return Print(format, reinterpret_cast<char*>(buffer), length, tailptr);
//}

//-----------------------------------------------------------------------------
// SystemLog::Pop
//
// Reads entries from the system log and removes them
//
// Arguments:
//
//	format		- Output format
//	buffer		- Output buffer
//	length		- Output buffer size, in bytes

//size_t SystemLog::Pop(SystemLogFormat format, void* buffer, size_t length)
//{
//	// Pop clears the entries as they are read, use member tail pointer
//	return Print(format, reinterpret_cast<char*>(buffer), length, m_tail);
//}

//-----------------------------------------------------------------------------
// SystemLog::Print (private)
//
// Prints from the tail of the system log, or counts spac required
//
// Arguments:
//
//	format		- Output string format
//	buffer		- Destination buffer, can be nullptr
//	length		- Length of the destination buffer
//	tailptr		- Buffer tail pointer to use for iteration

//size_t SystemLog::Print(SystemLogFormat format, char* buffer, size_t length, uintptr_t& tailptr)
//{
//	if(buffer == nullptr) length = MAXSIZE_T;			// Count only
//	size_t written = 0;									// Bytes written
//
//	// Iterate over the provided tail pointer and start dumping text
//	while((tailptr != m_head) && (length > 0)) {
//
//		// Write as much as possible into the output buffer; if the entry has been
//		// truncated, discard all characters after the previous entry and break;
//		size_t out = (format == SystemLogFormat::Device) ? 
//			PrintDeviceFormat(reinterpret_cast<LogEntry*>(tailptr), buffer, length) :
//			PrintStandardFormat(reinterpret_cast<LogEntry*>(tailptr), buffer, length);
//		if(out == _TRUNCATE) { if(buffer) *buffer = 0; break; }
//		
//		if(buffer) buffer += out;			// Increment the buffer pointer
//		length -= out;						// Reduce the amount of available space
//		written += out;						// Keep track of the total written
//		IncrementTailPointer(tailptr);		// Move to the next entry
//	}
//
//	// Add space for a final trailing null when just calculating length
//	return (buffer) ? written : written + 1;
//}

//-----------------------------------------------------------------------------
// SystemLog::PrintDeviceFormat (private)
//
// Prints the log entry in the /dev/kmsg character device format
//
// Arguments:
//
//	entry		- Current system log entry pointer
//	buffer		- Current pointer into the output buffer or NULL
//	length		- Space remaining in the output buffer

//size_t SystemLog::PrintDeviceFormat(LogEntry const* entry, char* buffer, size_t length)
//{
//	//
//	// TODO: THIS IS JUST STANDARD FORMAT -- IMPLEMENT IT
//	//
//
//	// Check that there is at least one byte of buffer left to write into
//	if(length == 0) return 0;
//
//	// Convert the timestamp into a double representing seconds from bias
//	double time = (entry->timestamp - m_tsbias) / m_tsfreq;
//
//	// Convert the priority code into a 16-bit unsigned integer, VC does not support %hhu format
//	uint16_t priority = (entry->facility << 3 | entry->level);
//
//	// Determine if the entry ends with a line feed character or not
//	bool haslinefeed = ((entry->messagelength) && (entry->message[entry->messagelength - 1] == '\n'));
//
//	// If a buffer was specified, format directly into it, otherwise just count the characters that would be required
//	return (buffer) ? _snprintf_s(buffer, length, _TRUNCATE, "<%hu>[%#12.06f] %.*s%s", priority, time, entry->messagelength, entry->message, (haslinefeed) ? "\0" : "\n") : 
//		_scprintf("<%hu>[%#12.06f] %.*s%s", priority, time, entry->messagelength, entry->message, (haslinefeed) ? "\0" : "\n");
//}

//-----------------------------------------------------------------------------
// SystemLog::PrintStandardFormat (private)
//
// Prints the log entry in the standard KMSG format
//
// Arguments:
//
//	entry		- Current system log entry pointer
//	buffer		- Current pointer into the output buffer or NULL
//	length		- Space remaining in the output buffer

//size_t SystemLog::PrintStandardFormat(LogEntry const* entry, char* buffer, size_t length)
//{
//	// Check that there is at least one byte of buffer left to write into
//	if(length == 0) return 0;
//
//	// Convert the timestamp into a double representing seconds from bias
//	double time = (entry->timestamp - m_tsbias) / m_tsfreq;
//
//	// Convert the priority code into a 16-bit unsigned integer, VC does not support %hhu format
//	uint16_t priority = (entry->facility << 3 | entry->level);
//
//	// Determine if the entry ends with a line feed character or not
//	bool haslinefeed = ((entry->messagelength) && (entry->message[entry->messagelength - 1] == '\n'));
//
//	// If a buffer was specified, format directly into it, otherwise just count the characters that would be required
//	return (buffer) ? _snprintf_s(buffer, length, _TRUNCATE, "<%hu>[%#12.06f] %.*s%s", priority, time, entry->messagelength, entry->message, (haslinefeed) ? "\0" : "\n") : 
//		_scprintf("<%hu>[%#12.06f] %.*s%s", priority, time, entry->messagelength, entry->message, (haslinefeed) ? "\0" : "\n");
//}

//-----------------------------------------------------------------------------
// SystemLog::SetDefaultLevel
//
// Changes the default message logging level
//
// Arguments:
//
//	level		- New default logging level to use

void SystemLog::SetDefaultLevel(enum class Level level)
{
	Capability::Demand(Capability::ConfigureSystemLog);

	// defaultlevel is stored as an atomic<>, don't take a lock
	if(level != Level::Default) m_defaultlevel = level;
}

//-----------------------------------------------------------------------------
// SystemLog::Write
//
// Writes a new log entry into the buffer
//
// Arguments:
//
//	facility	- Log entry facility code
//	level		- Log entry level code
//	message		- Message to be written
//	length		- Length, in characters, of the message

void SystemLog::Write(uint8_t facility, enum class Level level, char_t const* message, size_t length)
{
	length = std::min(length, MAX_MESSAGE);		// Truncate if > MAX_MESSAGE

	// Determine the overall aligned length of the log entry, must be <= 64KiB
	size_t entrylength = align::up(sizeof(entry_t) + length, __alignof(void*));
	_ASSERTE(entrylength <= UINT16_MAX);

	// The log write operation must be synchronized with any readers
	sync::reader_writer_lock::scoped_lock_write writer{ m_lock };

	// Check if writing this entry would wrap around to the top of the buffer
	if(m_head + entrylength > m_bottom) {

		// If the tail is currently at the top of the buffer, increment it
		if(m_tail == m_top) IncrementTailPointer(writer, m_tail);

		// Set all unused bytes at the end of the buffer to 0xFF and move head
		memset(reinterpret_cast<void*>(m_head), 0xFF, m_bottom - m_head);
		m_head = m_top;
	}

	// If the head pointer is behind the tail linearly, the tail may need to be
	// incremented until it's pushed out of the way of the new entry
	if(m_head < m_tail) {

		while((m_head != m_tail) && (m_head + entrylength > m_tail)) IncrementTailPointer(writer, m_tail);
	}

	// Write the entry into the buffer at the adjusted head position
	entry_t* entry = reinterpret_cast<entry_t*>(m_head);
	QueryPerformanceCounter(reinterpret_cast<PLARGE_INTEGER>(&entry->timestamp));
	
	entry->entrylength		= static_cast<uint16_t>(entrylength);
	entry->messagelength	= static_cast<uint16_t>(length);
	entry->facility			= facility;
	entry->level			= static_cast<uint8_t>(level);
	
	memcpy(entry->message, message, length);

	m_head += entrylength;			// Increment the head pointer
}

//---------------------------------------------------------------------------

#pragma warning(pop)
