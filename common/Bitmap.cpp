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
#include "Bitmap.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Bitmap Constructor
//
// Arguments:
//
//	bits		- Number of bits to define for a new, zero-initialized bitmap

Bitmap::Bitmap(uint32_t bits)
{
	// Determine the number of 32-bit ULONG elements required for the bitmap
	uint32_t ulongs = (align::up(bits, 32) >> 5);

	// Allocate the storage for the bitmap and zero-initialize it
	m_bitmap.Buffer = new ULONG[ulongs];
	if(m_bitmap.Buffer == nullptr) throw Exception(E_OUTOFMEMORY);

	// Initialize the bitmap bits to clear and set the allowable size
	memset(m_bitmap.Buffer, 0, sizeof(ULONG) * ulongs);
	m_bitmap.SizeOfBitMap = bits;
}

//-----------------------------------------------------------------------------
// Bitmap Copy Constructor

Bitmap::Bitmap(const Bitmap& rhs)
{
	// Determine the number of 32-bit ULONG elements required for the bitmap
	uint32_t ulongs = (align::up(rhs.m_bitmap.SizeOfBitMap, 32) >> 5);

	// Allocate the storage for the bitmap and zero-initialize it
	m_bitmap.Buffer = new ULONG[ulongs];
	if(m_bitmap.Buffer == nullptr) throw Exception(E_OUTOFMEMORY);

	// Copy the bitmap bits from the referenced object and set the size
	memcpy(&m_bitmap.Buffer, rhs.m_bitmap.Buffer, sizeof(ULONG) * ulongs);
	m_bitmap.SizeOfBitMap = rhs.m_bitmap.SizeOfBitMap;

	// Copy the hints from the referenced object
	m_clearhint = rhs.m_clearhint;
	m_sethint = rhs.m_sethint;
}

//-----------------------------------------------------------------------------
// Bitmap Move Constructor

Bitmap::Bitmap(Bitmap&& rhs) : m_bitmap(rhs.m_bitmap), m_clearhint(rhs.m_clearhint),
	m_sethint(rhs.m_sethint)
{
	rhs.m_bitmap.Buffer = nullptr;
	rhs.m_bitmap.SizeOfBitMap = 0;
}

//-----------------------------------------------------------------------------
// Bitmap Destructor

Bitmap::~Bitmap()
{
	// Release the ULONG buffer allocated for the bitmap
	if(m_bitmap.Buffer) delete[] m_bitmap.Buffer;
}

//-----------------------------------------------------------------------------
// Bitmap Assignmnent Operator

Bitmap& Bitmap::operator=(const Bitmap& rhs)
{
	// Determine the number of 32-bit ULONG elements required for the bitmap
	uint32_t ulongs = (align::up(rhs.m_bitmap.SizeOfBitMap, 32) >> 5);

	// If the bitmaps aren't the same length, it needs to be reallocated
	if(m_bitmap.SizeOfBitMap != rhs.m_bitmap.SizeOfBitMap) {

		// Release the old storage
		delete[] m_bitmap.Buffer;

		// Attempt to allocate new storage of the same length
		m_bitmap.Buffer = new ULONG[ulongs];
		if(m_bitmap.Buffer == nullptr) throw Exception(E_OUTOFMEMORY);

		// Set the new bitmap length to be the same
		m_bitmap.SizeOfBitMap = rhs.m_bitmap.SizeOfBitMap;
	}

	// Copy the bitmap bits from the referenced object
	memcpy(&m_bitmap.Buffer, rhs.m_bitmap.Buffer, sizeof(ULONG) * ulongs);

	// Copy the hints from the referenced object
	m_clearhint = rhs.m_clearhint;
	m_sethint = rhs.m_sethint;
	
	return *this;
}

//-----------------------------------------------------------------------------
// Bitmap::AreBitsClear
//
// Determines if all bits in a range are clear
//
// Arguments:
//
//	startbit		- Bit to start the range check against
//	count			- Number of bits in the range to verify

bool Bitmap::AreBitsClear(uint32_t startbit, uint32_t count)
{
	// Verify that the start bit combined with the count does not exceed the length
	if((startbit + count) >= m_bitmap.SizeOfBitMap) return false;

	// Test the requested range of bits in the bitmap
	return (NtApi::RtlAreBitsClear(&m_bitmap, startbit, count) == TRUE);
}

//-----------------------------------------------------------------------------
// Bitmap::AreBitsSet
//
// Determines if all bits in a range are set
//
// Arguments:
//
//	startbit		- Bit to start the range check against
//	count			- Number of bits in the range to verify

bool Bitmap::AreBitsSet(uint32_t startbit, uint32_t count)
{
	// Verify that the start bit combined with the count does not exceed the length
	if((startbit + count) >= m_bitmap.SizeOfBitMap) return false;

	// Test the requested range of bits in the bitmap
	return (NtApi::RtlAreBitsSet(&m_bitmap, startbit, count) == TRUE);
}

//-----------------------------------------------------------------------------
// Bitmap::Clear
//
// Clears all bits in the bitmap
//
// Arguments:
//
//	NONE

void Bitmap::Clear(void)
{
	// Clear all the bitmap bits and reset the hints
	NtApi::RtlClearAllBits(&m_bitmap);
	m_sethint = m_clearhint = 0;
}

//-----------------------------------------------------------------------------
// Bitmap::Clear
//
// Clears a single bit in the bitmap
//
// Arguments:
//
//	bit			- Index of the bit to be cleared

void Bitmap::Clear(uint32_t bit)
{
	// Verify the position is in bounds for the bitmap
	if(bit > m_bitmap.SizeOfBitMap) return;

	// Clear the bit and generate a new automatic hint
	NtApi::RtlClearBit(&m_bitmap, bit);
	m_sethint = (bit + 1);
}

//-----------------------------------------------------------------------------
// Bitmap::Clear
//
// Clears a range of bits in the bitmap
//
// Arguments:
//
//	startbit	- Starting bit index
//	count		- Number of bits to clear

void Bitmap::Clear(uint32_t startbit, uint32_t count)
{
	// Verify the starting bit is in range and don't allow count to overrun
	if(startbit > m_bitmap.SizeOfBitMap) return;
	count = min(count, (m_bitmap.SizeOfBitMap - startbit));

	// Clear the range of bits and generate a new automatic hint
	NtApi::RtlClearBits(&m_bitmap, startbit, count);
	m_sethint = (startbit + count);
}

//-----------------------------------------------------------------------------
// Bitmap::FindClear
//
// Locates a single clear bit in the bitmap
//
// Arguments:
//
//	NONE

uint32_t Bitmap::FindClear(void) 
{ 
	// Find a single clear bit using the automatic hint
	return FindClear(1, m_clearhint); 
}

//-----------------------------------------------------------------------------
// Bitmap::FindClear
//
// Locates a block of clear bits in the bitmap
//
// Arguments:
//
//	quantity	- Number of contiguous clear bits to locate

uint32_t Bitmap::FindClear(uint32_t quantity)
{ 
	// Find a range of clear bits using the automatic hint
	return FindClear(quantity, m_clearhint);
}
	
//-----------------------------------------------------------------------------
// Bitmap::FindClear
//
// Locates a block of clear bits in the bitmap
//
// Arguments:
//
//	quantity	- Number of contiguous clear bits to locate
//	hint		- Hint on where to start looking in the bitmap

uint32_t Bitmap::FindClear(uint32_t quantity, uint32_t hint)
{
	// Attempt to locate a range of clear bits with the specified quantity
	return NtApi::RtlFindClearBits(&m_bitmap, quantity, hint);
}
	
//-----------------------------------------------------------------------------
// Bitmap::FindClearAndSet
//
// Locates a single clear bit in the bitmap and sets it
//
// Arguments:
//
//	NONE

uint32_t Bitmap::FindClearAndSet(void) 
{ 
	// Set a single clear bit using the automatic hint
	return FindClearAndSet(1, m_clearhint); 
}

//-----------------------------------------------------------------------------
// Bitmap::FindClearAndSet
//
// Locates a block of clear bits in the bitmap and sets them
//
// Arguments:
//
//	quantity	- Number of contiguous bits to locate and set

uint32_t Bitmap::FindClearAndSet(uint32_t quantity) 
{
	// Set a range of clear bits using the automatic hint
	return FindClearAndSet(quantity, m_clearhint);
}

//-----------------------------------------------------------------------------
// Bitmap::FindClearAndSet
//
// Locates a block of clear bits in the bitmap and sets them, using a caller
// provided hint on where to start looking for the block
//
// Arguments:
//
//	quantity	- Number of contiguous bits to locate and set
//	hint		- Hint on where to start looking in the bitmap

uint32_t Bitmap::FindClearAndSet(uint32_t quantity, uint32_t hint)
{
	// Attempt to locate a range of clear bits with the specified quantity
	uint32_t result = NtApi::RtlFindClearBitsAndSet(&m_bitmap, quantity, hint);

	// If a range was set successfully, generate a new automatic hint
	if(result != 0xFFFFFFFF) m_clearhint = (result + quantity);
	return result;
}
	
//-----------------------------------------------------------------------------
// Bitmap::FindSet
//
// Locates a single set bit in the bitmap
//
// Arguments:
//
//	NONE

uint32_t Bitmap::FindSet(void) 
{ 
	// Find a single set bit using the automatic hint
	return FindSet(1, m_sethint); 
}

//-----------------------------------------------------------------------------
// Bitmap::FindSet
//
// Locates a block of set bits in the bitmap
//
// Arguments:
//
//	quantity	- Number of contiguous set bits to locate

uint32_t Bitmap::FindSet(uint32_t quantity)
{ 
	// Find a range of set bits using the automatic hint
	return FindSet(quantity, m_sethint);
}
	
//-----------------------------------------------------------------------------
// Bitmap::FindSet
//
// Locates a block of set bits in the bitmap
//
// Arguments:
//
//	quantity	- Number of contiguous set bits to locate
//	hint		- Hint on where to start looking in the bitmap

uint32_t Bitmap::FindSet(uint32_t quantity, uint32_t hint)
{
	// Attempt to locate a range of set bits with the specified quantity
	return NtApi::RtlFindSetBits(&m_bitmap, quantity, hint);
}
	
//-----------------------------------------------------------------------------
// Bitmap::FindSetAndClear
//
// Locates a single set bit in the bitmap and clears it
//
// Arguments:
//
//	NONE

uint32_t Bitmap::FindSetAndClear(void) 
{ 
	// Clear a single set bit using the automatic hint
	return FindSetAndClear(1, m_sethint);
}

//-----------------------------------------------------------------------------
// Bitmap::FindSetAndClear
//
// Locates a block of set bits in the bitmap and clears them
//
// Arguments:
//
//	quantity	- Number of contiguous bits to locate and clear

uint32_t Bitmap::FindSetAndClear(uint32_t quantity) 
{
	// Clear a range of set bits using the automatic hint
	return FindSetAndClear(quantity, m_sethint);
}

//-----------------------------------------------------------------------------
// Bitmap::FindSetAndClear
//
// Locates a block of set bits in the bitmap and clears them, using a caller
// provided hint on where to start looking for the block
//
// Arguments:
//
//	quantity	- Number of contiguous bits to locate and clear
//	hint		- Hint on where to start looking in the bitmap

uint32_t Bitmap::FindSetAndClear(uint32_t quantity, uint32_t hint)
{
	// Attempt to locate a range of set bits with the specified quantity
	uint32_t result = NtApi::RtlFindSetBitsAndClear(&m_bitmap, quantity, hint);

	// If a range was set successfully, update the automatic hint
	if(result != 0xFFFFFFFF) m_sethint = (result + quantity);
	return result;
}
	
//-----------------------------------------------------------------------------
// Bitmap::Set
//
// Sets all bits in the bitmap
//
// Arguments:
//
//	NONE

void Bitmap::Set(void)
{
	// Set all the bitmap bits and reset the hints back to zero
	NtApi::RtlSetAllBits(&m_bitmap);
	m_sethint = m_clearhint = 0;
}

//-----------------------------------------------------------------------------
// Bitmap::Set
//
// Sets a single bit in the bitmap
//
// Arguments:
//
//	bit			- Index of the bit to be set

void Bitmap::Set(uint32_t bit)
{
	// Verify the position is in bounds for the bitmap
	if(bit > m_bitmap.SizeOfBitMap) return;

	// Set the bit and generate a new automatic hint
	NtApi::RtlSetBit(&m_bitmap, bit);
	m_clearhint = (bit + 1);
}

//-----------------------------------------------------------------------------
// Bitmap::Set
//
// Sets a range of bits in the bitmap
//
// Arguments:
//
//	startbit	- Starting bit index
//	count		- Number of bits to set

void Bitmap::Set(uint32_t startbit, uint32_t count)
{
	// Verify the starting bit is in range and don't allow count to overrun
	if(startbit > m_bitmap.SizeOfBitMap) return;
	count = min(count, (m_bitmap.SizeOfBitMap - startbit));

	// Set the range of bits and generate a new automatic hint
	NtApi::RtlSetBits(&m_bitmap, startbit, count);
	m_clearhint = (startbit + count);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
