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

#ifndef __BITMAP_H_
#define __BITMAP_H_
#pragma once

#include "Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Bitmap
//
// Wrapper around the Windows RTL bitmap management functions.  Can be made
// more efficient as necessary by operating directly against the bitmap buffer
// rather than invoking the Windows API for everything.

class Bitmap
{
public:

	// Instance Constructor / Destructor
	//
	Bitmap(size_t bits);
	~Bitmap();
	
	//-------------------------------------------------------------------------
	// Member Functions

	// Clear
	//
	// Clears a specific bit, all bits, or a range of bits in the bitmap
	void Clear(void) { RtlClearAllBits(&m_bitmap); }
	void Clear(size_t bit) { if(bit <= m_bitmap.SizeOfBitMap) RtlClearBit(&m_bitmap, static_cast<ULONG>(bit)); }
	void Clear(size_t startbit, size_t count);

	// FindClear
	//
	// Finds a range of clear bits within the bitmap
	size_t FindClear(void) { return FindClear(1); }
	size_t FindClear(size_t quantity) { return static_cast<size_t>(RtlFindClearBits(&m_bitmap, static_cast<ULONG>(quantity), 0)); }

	// FindClearAndSet
	//
	// Finds a range of clear bits and sets them before returning
	size_t FindClearAndSet(void) { return FindClearAndSet(1); }
	size_t FindClearAndSet(size_t quantity) { return static_cast<size_t>(RtlFindClearBitsAndSet(&m_bitmap, static_cast<ULONG>(quantity), 0)); }

	// FindSet
	//
	// Finds a range of set bits within the bitmap
	size_t FindSet(void) { return FindSet(1); }
	size_t FindSet(size_t quantity) { return static_cast<size_t>(RtlFindSetBits(&m_bitmap, static_cast<ULONG>(quantity), 0)); }

	// FindSetAndClear
	//
	// Finds a range of set bits and clears them before returning
	size_t FindSetAndClear(void) { return FindSetAndClear(1); }
	size_t FindSetAndClear(size_t quantity) { return static_cast<size_t>(RtlFindSetBitsAndClear(&m_bitmap, static_cast<ULONG>(quantity), 0)); }

	// Set
	//
	// Sets a specific bit, all bits, or a range of bits in the bitmap
	void Set(void) { RtlSetAllBits(&m_bitmap); }
	void Set(size_t bit) { if(bit <= m_bitmap.SizeOfBitMap) RtlSetBit(&m_bitmap, static_cast<ULONG>(bit)); }
	void Set(size_t startbit, size_t count);

	//-------------------------------------------------------------------------
	// Fields

	// NotFound
	//
	// Return value from Find functions when specified range could not be found
	static const size_t NotFound = static_cast<size_t>(-1);

	//-------------------------------------------------------------------------
	// Properties

	//// Bit
	////
	//// Gets/sets the bit value at the specified location in the bitmap
	//__declspec(property(get=getBit, put=putBit)) bool Bit[];
	//bool getBit(size_t bit);
	//void putBit(size_t bit, bool value);

	// Empty
	//
	// Determines if the bitmap is empty
	__declspec(property(get=getEmpty)) bool Empty;
	bool getEmpty(void) { return RtlNumberOfSetBits(&m_bitmap) == 0; }

	// Full
	//
	// Determines if the bitmap is full
	__declspec(property(get=getFull)) bool Full;
	bool getFull(void) { return RtlNumberOfClearBits(&m_bitmap) == 0; }

private:

	Bitmap(const Bitmap&)=delete;
	Bitmap& operator=(const Bitmap&)=delete;
	
	//-------------------------------------------------------------------------
	// Private Type Declarations

	// RTL_BITMAP
	//
	// NTAPI structure not defined in the standard Win32 user-mode headers
	typedef struct _RTL_BITMAP {

		ULONG	SizeOfBitMap;			// Number of bits in bitmap
		PULONG	Buffer;					// Pointer to the bitmap itself
	
	} RTL_BITMAP, *PRTL_BITMAP;

	// RTL_BITMAP_RUN
	//
	// NTAPI structure not defined in the standard Win32 user-mode headers
	//typedef struct _RTL_BITMAP_RUN {

	//	ULONG	StartingIndex;
	//	ULONG	NumberOfBits;

	//} RTL_BITMAP_RUN, *PRTL_BITMAP_RUN;

	// NTAPI Functions
	//
	//using RtlAreBitsClearFunc				= BOOLEAN(NTAPI*)(PRTL_BITMAP, ULONG, ULONG);
	//using RtlAreBitsSetFunc					= BOOLEAN(NTAPI*)(PRTL_BITMAP, ULONG, ULONG);
	using RtlClearAllBitsFunc				= VOID(NTAPI*)(PRTL_BITMAP);
	using RtlClearBitFunc					= VOID(NTAPI*)(PRTL_BITMAP, ULONG);
	using RtlClearBitsFunc					= VOID(NTAPI*)(PRTL_BITMAP, ULONG, ULONG);
	using RtlFindClearBitsFunc				= ULONG(NTAPI*)(PRTL_BITMAP, ULONG, ULONG);
	using RtlFindClearBitsAndSetFunc		= ULONG(NTAPI*)(PRTL_BITMAP, ULONG, ULONG);
	//using RtlFindClearRunsFunc				= ULONG(NTAPI*)(PRTL_BITMAP, PRTL_BITMAP_RUN, ULONG, BOOLEAN);
	//using RtlFindFirstRunClearFunc			= ULONG(NTAPI*)(PRTL_BITMAP, PULONG);
	//using RtlFindLastBackwardRunClearFunc	= ULONG(NTAPI*)(PRTL_BITMAP, ULONG, PULONG);
	//using RtlFindLongestRunClearFunc		= ULONG(NTAPI*)(PRTL_BITMAP, PULONG);
	//using RtlFindNextForwardRunClearFunc	= ULONG(NTAPI*)(PRTL_BITMAP, ULONG, PULONG);
	using RtlFindSetBitsFunc				= ULONG(NTAPI*)(PRTL_BITMAP, ULONG, ULONG);
	using RtlFindSetBitsAndClearFunc		= ULONG(NTAPI*)(PRTL_BITMAP, ULONG, ULONG);
	//using RtlInitializeBitMapFunc			= VOID(NTAPI*)(PRTL_BITMAP, PULONG, ULONG);
	using RtlNumberOfClearBitsFunc			= ULONG(NTAPI*)(PRTL_BITMAP);
	//using RtlNumberOfClearBitsInRangeFunc	= ULONG(NTAPI*)(PRTL_BITMAP, ULONG, ULONG);
	using RtlNumberOfSetBitsFunc			= ULONG(NTAPI*)(PRTL_BITMAP);
	//using RtlNumberOfSetBitsInRangeFunc		= ULONG(NTAPI*)(PRTL_BITMAP, ULONG, ULONG);
	using RtlSetAllBitsFunc					= VOID(NTAPI*)(PRTL_BITMAP);
	using RtlSetBitFunc						= VOID(NTAPI*)(PRTL_BITMAP, ULONG);
	using RtlSetBitsFunc					= VOID(NTAPI*)(PRTL_BITMAP, ULONG, ULONG);
	//using RtlTestBitFunc					= BOOLEAN(NTAPI*)(PRTL_BITMAP, ULONG);

	//-------------------------------------------------------------------------
	// Member Variables

	RTL_BITMAP			m_bitmap;			// Contained RTL bitmap struct

	// NTAPI
	//
	//static RtlAreBitsClearFunc				RtlAreBitsClear;
	//static RtlAreBitsSetFunc				RtlAreBitsSet;
	static RtlClearAllBitsFunc				RtlClearAllBits;
	static RtlClearBitFunc					RtlClearBit;
	static RtlClearBitsFunc					RtlClearBits;
	static RtlFindClearBitsFunc				RtlFindClearBits;
	static RtlFindClearBitsAndSetFunc		RtlFindClearBitsAndSet;
	//static RtlFindClearRunsFunc				RtlFindClearRuns;
	//static RtlFindFirstRunClearFunc			RtlFindFirstRunClear;
	//static RtlFindLastBackwardRunClearFunc	RtlFindLastBackwardRunClear;
	//static RtlFindLongestRunClearFunc		RtlFindLongestRunClear;
	//static RtlFindNextForwardRunClearFunc	RtlFindNextForwardRunClear;
	static RtlFindSetBitsFunc				RtlFindSetBits;
	static RtlFindSetBitsAndClearFunc		RtlFindSetBitsAndClear;
	//static RtlInitializeBitMapFunc			RtlInitializeBitMap;
	static RtlNumberOfClearBitsFunc			RtlNumberOfClearBits;
	//static RtlNumberOfClearBitsInRangeFunc	RtlNumberOfClearBitsInRange;
	static RtlNumberOfSetBitsFunc			RtlNumberOfSetBits;
	//static RtlNumberOfSetBitsInRangeFunc	RtlNumberOfSetBitsInRange;
	static RtlSetAllBitsFunc				RtlSetAllBits;
	static RtlSetBitFunc					RtlSetBit;
	static RtlSetBitsFunc					RtlSetBits;
	//static RtlTestBitFunc					RtlTestBit;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __BITMAP_H_
