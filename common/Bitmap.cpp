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
#include "Bitmap.h"

#pragma warning(push, 4)

// Bitmap::RtlAreBitsClear
//
//Bitmap::RtlAreBitsClearFunc Bitmap::RtlAreBitsClear = 
//reinterpret_cast<RtlAreBitsClearFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlAreBitsClear"); 
//}());

// Bitmap::RtlAreBitsSet
//
//Bitmap::RtlAreBitsSetFunc Bitmap::RtlAreBitsSet = 
//reinterpret_cast<RtlAreBitsSetFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlAreBitsSet"); 
//}());

// Bitmap::RtlClearAllBits
//
Bitmap::RtlClearAllBitsFunc Bitmap::RtlClearAllBits = 
reinterpret_cast<RtlClearAllBitsFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlClearAllBits"); 
}());

// Bitmap::RtlClearBit
//
Bitmap::RtlClearBitFunc Bitmap::RtlClearBit = 
reinterpret_cast<RtlClearBitFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlClearBit"); 
}());

// Bitmap::RtlClearBits
//
Bitmap::RtlClearBitsFunc Bitmap::RtlClearBits = 
reinterpret_cast<RtlClearBitsFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlClearBits"); 
}());

// Bitmap::RtlFindClearBits
//
Bitmap::RtlFindClearBitsFunc Bitmap::RtlFindClearBits = 
reinterpret_cast<RtlFindClearBitsFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlFindClearBits"); 
}());

// Bitmap::RtlFindClearBitsAndSet
//
Bitmap::RtlFindClearBitsAndSetFunc Bitmap::RtlFindClearBitsAndSet = 
reinterpret_cast<RtlFindClearBitsAndSetFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlFindClearBitsAndSet"); 
}());

// Bitmap::RtlFindClearRuns
//
//Bitmap::RtlFindClearRunsFunc Bitmap::RtlFindClearRuns = 
//reinterpret_cast<RtlFindClearRunsFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlFindClearRuns"); 
//}());

// Bitmap::RtlFindFirstRunClear
//
//Bitmap::RtlFindFirstRunClearFunc Bitmap::RtlFindFirstRunClear = 
//reinterpret_cast<RtlFindFirstRunClearFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlFindFirstRunClear"); 
//}());

// Bitmap::RtlFindLastBackwardRunClear
//
//Bitmap::RtlFindLastBackwardRunClearFunc Bitmap::RtlFindLastBackwardRunClear = 
//reinterpret_cast<RtlFindLastBackwardRunClearFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlFindLastBackwardRunClear"); 
//}());

// Bitmap::RtlFindLongestRunClear
//
//Bitmap::RtlFindLongestRunClearFunc Bitmap::RtlFindLongestRunClear = 
//reinterpret_cast<RtlFindLongestRunClearFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlFindLongestRunClear"); 
//}());

// Bitmap::RtlFindNextForwardRunClear
//
//Bitmap::RtlFindNextForwardRunClearFunc Bitmap::RtlFindNextForwardRunClear = 
//reinterpret_cast<RtlFindNextForwardRunClearFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlFindNextForwardRunClear"); 
//}());

// Bitmap::RtlFindSetBits
//
Bitmap::RtlFindSetBitsFunc Bitmap::RtlFindSetBits = 
reinterpret_cast<RtlFindSetBitsFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlFindSetBits"); 
}());

// Bitmap::RtlFindSetBitsAndClear
//
Bitmap::RtlFindSetBitsAndClearFunc Bitmap::RtlFindSetBitsAndClear = 
reinterpret_cast<RtlFindSetBitsAndClearFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlFindSetBitsAndClear"); 
}());

// Bitmap::RtlInitializeBitMap
//
//Bitmap::RtlInitializeBitMapFunc Bitmap::RtlInitializeBitMap = 
//reinterpret_cast<RtlInitializeBitMapFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlInitializeBitMap"); 
//}());

// Bitmap::RtlNumberOfClearBits
//
Bitmap::RtlNumberOfClearBitsFunc Bitmap::RtlNumberOfClearBits = 
reinterpret_cast<RtlNumberOfClearBitsFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlNumberOfClearBits"); 
}());

//// Bitmap::RtlNumberOfClearBitsInRange
////
//Bitmap::RtlNumberOfClearBitsInRangeFunc Bitmap::RtlNumberOfClearBitsInRange = 
//reinterpret_cast<RtlNumberOfClearBitsInRangeFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlNumberOfClearBitsInRange"); 
//}());

// Bitmap::RtlNumberOfSetBits
//
Bitmap::RtlNumberOfSetBitsFunc Bitmap::RtlNumberOfSetBits = 
reinterpret_cast<RtlNumberOfSetBitsFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlNumberOfSetBits"); 
}());

//// Bitmap::RtlNumberOfSetBitsInRange
////
//Bitmap::RtlNumberOfSetBitsInRangeFunc Bitmap::RtlNumberOfSetBitsInRange = 
//reinterpret_cast<RtlNumberOfSetBitsInRangeFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlNumberOfSetBitsInRange"); 
//}());

// Bitmap::RtlSetAllBits
//
Bitmap::RtlSetAllBitsFunc Bitmap::RtlSetAllBits = 
reinterpret_cast<RtlSetAllBitsFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlSetAllBits"); 
}());

// Bitmap::RtlSetBit
//
Bitmap::RtlSetBitFunc Bitmap::RtlSetBit = 
reinterpret_cast<RtlSetBitFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlSetBit"); 
}());

// Bitmap::RtlSetBits
//
Bitmap::RtlSetBitsFunc Bitmap::RtlSetBits = 
reinterpret_cast<RtlSetBitsFunc>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlSetBits"); 
}());

// Bitmap::RtlTestBit
//
//Bitmap::RtlTestBitFunc Bitmap::RtlTestBit = 
//reinterpret_cast<RtlTestBitFunc>([]() -> FARPROC { 
//	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "RtlTestBit"); 
//}());

//-----------------------------------------------------------------------------
// Bitmap Constructor
//
// Arguments:
//
//	bits		- Number of bits to define for a new, zero-initialized bitmap

Bitmap::Bitmap(size_t bits)
{
	// The RTL_BITMAP structure is supposed to be opaque, but since it needed to
	// be declared locally there is no reason to bother with creating duplicate
	// member variables and passing them into RtlInitializeBitMap()

	// Determine the number of 32-bit ULONG elements required for the bitmap
	size_t ulongs = (align::up(bits, 32) >> 5);

	// Allocate the storage for the bitmap and zero-initialize it
	m_bitmap.Buffer = new ULONG[ulongs];
	if(m_bitmap.Buffer == nullptr) throw Exception(E_OUTOFMEMORY);

	// Initialize the bitmap bits to clear and set the allowable size
	memset(m_bitmap.Buffer, 0, sizeof(ULONG) * ulongs);
	m_bitmap.SizeOfBitMap = bits;
}

//-----------------------------------------------------------------------------
// Bitmap Destructor

Bitmap::~Bitmap()
{
	if(m_bitmap.Buffer) delete[] m_bitmap.Buffer;
}

//-----------------------------------------------------------------------------
// Bitmap::getBit

//bool Bitmap::getBit(size_t index)
//{
//	// Ensure that the index does not overrun the contained bitmap
//	if(index > m_bitmap.SizeOfBitMap) return false;
//
//	return (RtlTestBit(&m_bitmap, static_cast<ULONG>(index)) == TRUE);
//}

//-----------------------------------------------------------------------------
// Bitmap::putBit

//void Bitmap::putBit(size_t index, bool value)
//{
//	// Ensure that the index does not overrun the contained bitmap
//	if(index > m_bitmap.SizeOfBitMap) return;
//
//	// Either set or clear the bit at the specified location
//	if(value) RtlSetBit(&m_bitmap, static_cast<ULONG>(index));
//	else RtlClearBit(&m_bitmap, static_cast<ULONG>(index));
//}

//-----------------------------------------------------------------------------
// Bitmap::Clear
//
// Clears a range of bits in the bitmap
//
// Arguments:
//
//	startbit		- Starting bit index
//	count			- Number of bits to clear

void Bitmap::Clear(size_t startbit, size_t count)
{
	// Verify the starting bit is in range and don't allow count to overrun
	if(startbit > m_bitmap.SizeOfBitMap) return;
	count = min(count, (m_bitmap.SizeOfBitMap - startbit));

	RtlClearBits(&m_bitmap, static_cast<ULONG>(startbit), static_cast<ULONG>(count));
}

//-----------------------------------------------------------------------------
// Bitmap::Set
//
// Sets a range of bits in the bitmap
//
// Arguments:
//
//	startbit		- Starting bit index
//	count			- Number of bits to set

void Bitmap::Set(size_t startbit, size_t count)
{
	// Verify the starting bit is in range and don't allow count to overrun
	if(startbit > m_bitmap.SizeOfBitMap) return;
	count = min(count, (m_bitmap.SizeOfBitMap - startbit));

	RtlSetBits(&m_bitmap, static_cast<ULONG>(startbit), static_cast<ULONG>(count));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
