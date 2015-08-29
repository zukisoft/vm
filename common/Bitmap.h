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

#ifndef __BITMAP_H_
#define __BITMAP_H_
#pragma once

#include <stdint.h>
#include "NtApi.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Bitmap
//
// Wrapper around the Windows RTL bitmap management functions. Note that these 
// functions are not natively thread-safe, so this class must be externally 
// protected from concurrent access

class Bitmap final
{
public:

	// Constructors
	//
	Bitmap(uint32_t bits);
	Bitmap(const Bitmap& rhs);
	Bitmap(Bitmap&& rhs);

	// Destructor
	//
	~Bitmap();

	// Assignment Operator
	//
	Bitmap& operator=(const Bitmap& rhs);

	// Array subscript operator
	//
	bool operator[](uint32_t bit) const;

	//-------------------------------------------------------------------------
	// Member Functions

	// AreBitsClear
	//
	// Determines if a range of bits is clear
	bool AreBitsClear(uint32_t startbit, uint32_t count) const;

	// AreBitsSet
	//
	// Determines if a range of bits is set
	bool AreBitsSet(uint32_t startbit, uint32_t count) const;

	// Clear
	//
	// Clears a specific bit in the bitmap
	void Clear(uint32_t bit);

	// Clear
	//
	// Clears a range of bits in the bitmap
	void Clear(uint32_t startbit, uint32_t count);

	// ClearAll
	//
	// Clears all bits in the bitmap
	void ClearAll(void);

	// FindClear
	//
	// Finds a single clear bit in the bitmap
	uint32_t FindClear(void) const;

	// FindClear
	//
	// Finds a range of clear bits in the bitmap
	uint32_t FindClear(uint32_t quantity) const;

	// FindClear
	//
	// Finds a range of clear bits in the bitmap
	uint32_t FindClear(uint32_t quantity, uint32_t hint) const;

	// FindClearAndSet
	//
	// Finds a single clear bit in the bitmap and sets it
	uint32_t FindClearAndSet(void);

	// FindClearAndSet
	//
	// Finds a range of clear bits in the bitmap and sets them
	uint32_t FindClearAndSet(uint32_t quantity);

	// FindClearAndSet
	//
	// Finds a range of clear bits in the bitmap and sets them
	uint32_t FindClearAndSet(uint32_t quantity, uint32_t hint);

	// FindSet
	//
	// Finds a single set bit in the bitmap
	uint32_t FindSet(void) const;

	// FindSet
	//
	// Finds a range of set bits in the bitmap
	uint32_t FindSet(uint32_t quantity) const;

	// FindSet
	//
	// Finds a range of set bits in the bitmap
	uint32_t FindSet(uint32_t quantity, uint32_t hint) const;

	// FindSetAndClear
	//
	// Finds a single set bit in the bitmap and clears it
	uint32_t FindSetAndClear(void);

	// FindSetAndClear
	//
	// Finds a range of set bits in the bitmap and clears them
	uint32_t FindSetAndClear(uint32_t quantity);

	// FindSetAndClear
	//
	// Finds a range of set bits in the bitmap and clears them
	uint32_t FindSetAndClear(uint32_t quantity, uint32_t hint);

	// Set
	//
	// Sets a specific bit in the bitmap
	void Set(uint32_t bit);

	// Set
	//
	// Sets a range of bits in the bitmap
	void Set(uint32_t startbit, uint32_t count);

	// SetAll
	//
	// Sets all bits in the bitmap
	void SetAll(void);

	//-------------------------------------------------------------------------
	// Fields

	// NotFound
	//
	// Return value from Find functions when specified range could not be found
	static const uint32_t NotFound{ 0xFFFFFFFFui32 };

	//-------------------------------------------------------------------------
	// Properties

	// Available
	//
	// Gets the number of available bits in the bitmap
	__declspec(property(get=getAvailable)) uint32_t Available;
	uint32_t getAvailable(void) const;

	// Consumed
	//
	// Gets the number of consumed bits in the bitmap
	__declspec(property(get=getConsumed)) uint32_t Consumed;
	uint32_t getConsumed(void) const;

	// Empty
	//
	// Determines if the bitmap is empty
	__declspec(property(get=getEmpty)) bool Empty;
	bool getEmpty(void) const;

	// Full
	//
	// Determines if the bitmap is full
	__declspec(property(get=getFull)) bool Full;
	bool getFull(void) const;

	// Size
	//
	// Gets/sets the size of the bitmap
	__declspec(property(get=getSize, put=putSize)) uint32_t Size;
	uint32_t getSize(void) const;
	void putSize(uint32_t value);

private:
	
	//-------------------------------------------------------------------------
	// Member Variables

	NtApi::RTL_BITMAP	m_bitmap;			// Contained RTL bitmap struct
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __BITMAP_H_
