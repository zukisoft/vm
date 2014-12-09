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

#include "stdafx.h"
#include "SystemLog.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SystemLog Constructor
//
// Arguments:
//
//	size		- Size of the system log ring buffer

SystemLog::SystemLog(size_t size)
{
	// Minimum log size is the page size, maximum is constant MAX_BUFFER
	if(size < SystemInformation::PageSize) size = SystemInformation::PageSize;
	else if(size > MAX_BUFFER) size = MAX_BUFFER;

	// Attempt to allocate the log buffer from virtual memory
	m_buffer = MemoryRegion::Reserve(size, MEM_COMMIT);

	// Initialize the top, head and tail pointers to the top of the buffer
	m_top = m_head = m_tail = uintptr_t(m_buffer->Pointer);

	// Knowing the bottom of the buffer without calculating it is useful
	m_bottom = m_top + m_buffer->Length;

	// Get the timestamp frequency in counts per second and convert to double
	LARGE_INTEGER tsfreq;
	QueryPerformanceFrequency(&tsfreq);
	m_tsfreq = static_cast<double>(tsfreq.QuadPart);
}

//-----------------------------------------------------------------------------
// SystemLog::IncrementTailPointer (private)
//
// Increments a tail pointer to point at the next system log entry
//
// Arguments:
//
//	tailptr		- Tail pointer to be incremented

bool SystemLog::IncrementTailPointer(uintptr_t& tailptr)
{
	// If the tail is in the same position as the head, the buffer is empty
	if(tailptr == m_head) return false;

	// Get the length of the entry currently being pointed to
	uint16_t entrylength = reinterpret_cast<LogEntry*>(tailptr)->entrylength;

	// Increment the tail to point at the next entry in the buffer  
	tailptr += entrylength;

	// If there isn't enough room left to hold another entry header, or the length 
	// has been set to 0xFFFF, move the tail pointer back to the top of the buffer
	if((tailptr + sizeof(LogEntry) > m_bottom) || (entrylength == UINT16_MAX)) tailptr = m_top;

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

size_t SystemLog::Peek(SystemLogFormat format, void* buffer, size_t length)
{
	// Peek is non-destructive, clone the member tail pointer
	uintptr_t tailptr = m_tail;
	return Print(format, reinterpret_cast<char*>(buffer), length, tailptr);
}

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

size_t SystemLog::Pop(SystemLogFormat format, void* buffer, size_t length)
{
	// Pop clears the entries as they are read, use member tail pointer
	return Print(format, reinterpret_cast<char*>(buffer), length, m_tail);
}

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

size_t SystemLog::Print(SystemLogFormat format, char* buffer, size_t length, uintptr_t& tailptr)
{
	if(buffer == nullptr) length = MAXSIZE_T;			// Count only
	size_t written = 0;									// Bytes written

	// Iterate over the provided tail pointer and start dumping text
	while((tailptr != m_head) && (length > 0)) {

		// Write as much as possible into the output buffer; if the entry has been
		// truncated, discard all characters after the previous entry and break;
		size_t out = (format == SystemLogFormat::Device) ? 
			PrintDeviceFormat(reinterpret_cast<LogEntry*>(tailptr), buffer, length) :
			PrintStandardFormat(reinterpret_cast<LogEntry*>(tailptr), buffer, length);
		if(out == _TRUNCATE) { if(buffer) *buffer = 0; break; }
		
		if(buffer) buffer += out;			// Increment the buffer pointer
		length -= out;						// Reduce the amount of available space
		written += out;						// Keep track of the total written
		IncrementTailPointer(tailptr);		// Move to the next entry
	}

	// Add space for a final trailing null when just calculating length
	return (buffer) ? written : written + 1;
}

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

size_t SystemLog::PrintDeviceFormat(const LogEntry* entry, char* buffer, size_t length)
{
	//
	// TODO: THIS IS JUST STANDARD FORMAT -- IMPLEMENT IT
	//

	// Check that there is at least one byte of buffer left to write into
	if(length == 0) return 0;

	// Convert the timestamp into a double representing seconds from bias
	double time = (entry->timestamp - m_tsbias) / m_tsfreq;

	// Convert the priority code into a 16-bit unsigned integer, VC does not support %hhu format
	uint16_t priority = (entry->facility << 3 | entry->level);

	// Determine if the entry ends with a line feed character or not
	bool haslinefeed = ((entry->messagelength) && (entry->message[entry->messagelength - 1] == '\n'));

	// If a buffer was specified, format directly into it, otherwise just count the characters that would be required
	return (buffer) ? _snprintf_s(buffer, length, _TRUNCATE, "<%hu>[%#12.06f] %.*s%s", priority, time, entry->messagelength, entry->message, (haslinefeed) ? "\0" : "\n") : 
		_scprintf("<%hu>[%#12.06f] %.*s%s", priority, time, entry->messagelength, entry->message, (haslinefeed) ? "\0" : "\n");
}

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

size_t SystemLog::PrintStandardFormat(const LogEntry* entry, char* buffer, size_t length)
{
	// Check that there is at least one byte of buffer left to write into
	if(length == 0) return 0;

	// Convert the timestamp into a double representing seconds from bias
	double time = (entry->timestamp - m_tsbias) / m_tsfreq;

	// Convert the priority code into a 16-bit unsigned integer, VC does not support %hhu format
	uint16_t priority = (entry->facility << 3 | entry->level);

	// Determine if the entry ends with a line feed character or not
	bool haslinefeed = ((entry->messagelength) && (entry->message[entry->messagelength - 1] == '\n'));

	// If a buffer was specified, format directly into it, otherwise just count the characters that would be required
	return (buffer) ? _snprintf_s(buffer, length, _TRUNCATE, "<%hu>[%#12.06f] %.*s%s", priority, time, entry->messagelength, entry->message, (haslinefeed) ? "\0" : "\n") : 
		_scprintf("<%hu>[%#12.06f] %.*s%s", priority, time, entry->messagelength, entry->message, (haslinefeed) ? "\0" : "\n");
}

//-----------------------------------------------------------------------------
// SystemLog::Push
//
// Pushes a new log entry into the buffer
//
// Arguments:
//
//	level		- Log entry level code
//	facility	- Log entry facility code
//	message		- Message to be pushed

void SystemLog::Push(SystemLogLevel level, SystemLogFacility facility, const char_t* message)
{
	// The maximum message length is UINT16_MAX (64KiB)
	size_t messagelength = min(strlen(message) * sizeof(char_t), UINT16_MAX);

	// Determine the length of the entire log entry, and discard it if > UINT16_MAX
	size_t entrylength = align::up(sizeof(LogEntry) + messagelength, __alignof(void*));
	if(entrylength == UINT16_MAX) return;

	// Check if writing this entry would wrap around to the top of the buffer
	if(m_head + entrylength > m_bottom) {

		// If the tail is currently at the top of the buffer, increment it
		if(m_tail == m_top) IncrementTailPointer(m_tail);

		// Set all unused bytes at the end of the buffer to 0xFF and move head
		memset(reinterpret_cast<void*>(m_head), 0xFF, m_bottom - m_head);
		m_head = m_top;
	}

	// If the head pointer is behind the tail linearly, the tail may need to be
	// incremented until it's pushed out of the way of the new entry
	if(m_head < m_tail) 
		while((m_head != m_tail) && (m_head + entrylength > m_tail)) IncrementTailPointer(m_tail);

	// Write the entry into the buffer at the adjusted head position
	LogEntry* entry = reinterpret_cast<LogEntry*>(m_head);
	QueryPerformanceCounter(reinterpret_cast<PLARGE_INTEGER>(&entry->timestamp));
	entry->entrylength = static_cast<uint16_t>(entrylength);
	entry->messagelength = static_cast<uint16_t>(messagelength);
	entry->facility = static_cast<uint8_t>(facility);
	entry->level = static_cast<uint8_t>(level);
	memcpy(entry->message, message, messagelength);

	m_head += entrylength;			// Increment the head pointer
}

//---------------------------------------------------------------------------

#pragma warning(pop)
