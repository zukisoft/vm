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

Bitmap::Bitmap(uint32_t bits) : m_bitmap({ 0, nullptr })
{
	// Allocate a zero-initialized buffer for the bitmap off the process heap; the buffer size
	// must be a multiple of 32 bits for compatibility with the bitmap API
	m_bitmap.Buffer = reinterpret_cast<PULONG>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, align::up(bits, 32) >> 3));
	if(!m_bitmap.Buffer) throw Exception(E_OUTOFMEMORY);

	m_bitmap.SizeOfBitMap = bits;		// Bitmap was successfully allocated
}

//-----------------------------------------------------------------------------
// Bitmap Copy Constructor

Bitmap::Bitmap(const Bitmap& rhs) : m_bitmap({ 0, nullptr })
{
	// Determine the size of the referenced object's bitmap buffer, in bytes
	size_t size = align::up(rhs.m_bitmap.SizeOfBitMap, 32) >> 3;

	// Allocate a new heap buffer to contain the copy of the referenced object
	m_bitmap.Buffer = reinterpret_cast<PULONG>(HeapAlloc(GetProcessHeap(), 0, size));
	if(m_bitmap.Buffer == nullptr) throw Exception(E_OUTOFMEMORY);

	// Copy the bitmap bits and size from the referenced object
	memcpy(m_bitmap.Buffer, rhs.m_bitmap.Buffer, size);
	m_bitmap.SizeOfBitMap = rhs.m_bitmap.SizeOfBitMap;
}

//-----------------------------------------------------------------------------
// Bitmap Move Constructor

Bitmap::Bitmap(Bitmap&& rhs) : m_bitmap(rhs.m_bitmap)
{
	rhs.m_bitmap.Buffer = nullptr;
	rhs.m_bitmap.SizeOfBitMap = 0;
}

//-----------------------------------------------------------------------------
// Bitmap Destructor

Bitmap::~Bitmap()
{
	if(m_bitmap.Buffer) HeapFree(GetProcessHeap(), 0, m_bitmap.Buffer);
}

//-----------------------------------------------------------------------------
// Bitmap::operator=

Bitmap& Bitmap::operator=(const Bitmap& rhs)
{
	// Determine the size of the referenced object's bitmap buffer, in bytes
	size_t size = align::up(rhs.m_bitmap.SizeOfBitMap, 32) >> 3;

	// Reallocate the heap buffer to match the referenced object's buffer size
	void* newbuffer = HeapReAlloc(GetProcessHeap(), 0, m_bitmap.Buffer, size);
	if(newbuffer == nullptr) throw Exception(E_OUTOFMEMORY);

	// Copy the bits from the referenced object into the reallocated buffer
	memcpy(newbuffer, rhs.m_bitmap.Buffer, size);

	// Assign the updated buffer pointer and bitmap size information
	m_bitmap.Buffer = reinterpret_cast<PULONG>(newbuffer);
	m_bitmap.SizeOfBitMap = rhs.m_bitmap.SizeOfBitMap;

	return *this;
}

//-----------------------------------------------------------------------------
// Bitmap::operator[]

bool Bitmap::operator[](uint32_t bit) const
{
	return (bit < m_bitmap.SizeOfBitMap) ? NtApi::RtlTestBit(&m_bitmap, bit) == TRUE : false;
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

bool Bitmap::AreBitsClear(uint32_t startbit, uint32_t count) const
{
	// Verify that the start bit combined with the count does not exceed the length
	if((startbit + count) > m_bitmap.SizeOfBitMap) return false;

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

bool Bitmap::AreBitsSet(uint32_t startbit, uint32_t count) const
{
	// Verify that the start bit combined with the count does not exceed the length
	if((startbit + count) > m_bitmap.SizeOfBitMap) return false;

	// Test the requested range of bits in the bitmap
	return (NtApi::RtlAreBitsSet(&m_bitmap, startbit, count) == TRUE);
}

//-----------------------------------------------------------------------------
// Bitmap::getAvailable
//
// Gets the number of available bits in the bitmap

uint32_t Bitmap::getAvailable(void) const
{
	return NtApi::RtlNumberOfClearBits(&m_bitmap);
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
	if(bit < m_bitmap.SizeOfBitMap) NtApi::RtlClearBit(&m_bitmap, bit);
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
	if(startbit < m_bitmap.SizeOfBitMap) 
		NtApi::RtlClearBits(&m_bitmap, startbit, std::min(static_cast<ULONG>(count), m_bitmap.SizeOfBitMap - startbit));
}

//-----------------------------------------------------------------------------
// Bitmap::ClearAll
//
// Clears all bits in the bitmap
//
// Arguments:
//
//	NONE

void Bitmap::ClearAll(void)
{
	NtApi::RtlClearAllBits(&m_bitmap);
}

//-----------------------------------------------------------------------------
// Bitmap::getConsumed
//
// Gets the number of bits consumed in the bitmap

uint32_t Bitmap::getConsumed(void) const
{
	return NtApi::RtlNumberOfSetBits(&m_bitmap);
}

//-----------------------------------------------------------------------------
// Bitmap::getEmpty
//
// Determines if the bitmap is empty

bool Bitmap::getEmpty(void) const
{
	return NtApi::RtlNumberOfSetBits(&m_bitmap) == 0;
}

//-----------------------------------------------------------------------------
// Bitmap::FindClear
//
// Locates a single clear bit in the bitmap
//
// Arguments:
//
//	NONE

uint32_t Bitmap::FindClear(void) const
{ 
	return NtApi::RtlFindClearBits(&m_bitmap, 1, 0);
}

//-----------------------------------------------------------------------------
// Bitmap::FindClear
//
// Locates a block of clear bits in the bitmap
//
// Arguments:
//
//	quantity	- Number of contiguous clear bits to locate

uint32_t Bitmap::FindClear(uint32_t quantity) const
{ 
	return NtApi::RtlFindClearBits(&m_bitmap, quantity, 0);
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

uint32_t Bitmap::FindClear(uint32_t quantity, uint32_t hint) const
{
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
	return NtApi::RtlFindClearBitsAndSet(&m_bitmap, 1, 0);
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
	return NtApi::RtlFindClearBitsAndSet(&m_bitmap, quantity, 0);
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
	return NtApi::RtlFindClearBitsAndSet(&m_bitmap, quantity, hint);
}
	
//-----------------------------------------------------------------------------
// Bitmap::FindSet
//
// Locates a single set bit in the bitmap
//
// Arguments:
//
//	NONE

uint32_t Bitmap::FindSet(void) const
{ 
	return NtApi::RtlFindSetBits(&m_bitmap, 1, 0);
}

//-----------------------------------------------------------------------------
// Bitmap::FindSet
//
// Locates a block of set bits in the bitmap
//
// Arguments:
//
//	quantity	- Number of contiguous set bits to locate

uint32_t Bitmap::FindSet(uint32_t quantity) const
{ 
	return NtApi::RtlFindSetBits(&m_bitmap, quantity, 0);
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

uint32_t Bitmap::FindSet(uint32_t quantity, uint32_t hint) const
{
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
	return NtApi::RtlFindSetBitsAndClear(&m_bitmap, 1, 0);
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
	return NtApi::RtlFindSetBitsAndClear(&m_bitmap, quantity, 0);
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
	return NtApi::RtlFindSetBitsAndClear(&m_bitmap, quantity, hint);
}
	
//-----------------------------------------------------------------------------
// Bitmap::getFull
//
// Determines if the bitmap is full

bool Bitmap::getFull(void) const
{
	return NtApi::RtlNumberOfClearBits(&m_bitmap) == 0;
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
	if(bit < m_bitmap.SizeOfBitMap) NtApi::RtlSetBit(&m_bitmap, bit);
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
	if(startbit < m_bitmap.SizeOfBitMap)
		NtApi::RtlSetBits(&m_bitmap, startbit, std::min(static_cast<ULONG>(count), m_bitmap.SizeOfBitMap - startbit));
}

//-----------------------------------------------------------------------------
// Bitmap::SetAll
//
// Sets all bits in the bitmap
//
// Arguments:
//
//	NONE

void Bitmap::SetAll(void)
{
	NtApi::RtlSetAllBits(&m_bitmap);
}

//-----------------------------------------------------------------------------
// Bitmap::getSize
//
// Gets the size of the bitmap

uint32_t Bitmap::getSize(void) const
{
	return m_bitmap.SizeOfBitMap;
}

//-----------------------------------------------------------------------------
// Bitmap::putSize
//
// Sets the size of the bitmap

void Bitmap::putSize(uint32_t bits)
{
	uint32_t aligned = align::up(bits, 32);		// Align to ULONG boundary

	// Reallocate the heap buffer to match the requested new aligned size
	void* newbuffer = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_bitmap.Buffer, aligned >> 3);
	if(newbuffer == nullptr) throw Exception(E_OUTOFMEMORY);

	// Replace the original pointer and length with the aligned buffer
	m_bitmap.Buffer = reinterpret_cast<PULONG>(newbuffer);
	m_bitmap.SizeOfBitMap = aligned;

	// Clear any bits that lie between the unaligned and aligned length
	// prior to assigning the final requested bitmap size
	if(bits != aligned) {

		NtApi::RtlClearBits(&m_bitmap, bits, aligned - bits);
		m_bitmap.SizeOfBitMap = bits;
	}
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
