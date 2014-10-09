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

#ifndef __MODRM_H_
#define __MODRM_H_
#pragma once

#include "ContextRecord.h"

#pragma warning(push, 4)				
#pragma warning(disable:4201)			// Nameless struct/union

//-----------------------------------------------------------------------------
// ModRM
//
// Union that defines the values within a ModR/M byte and provides the means
// to calcuate the effective address that it specified

class ModRM 
{
public:
	
	// Instance Constructors
	//
	explicit ModRM(uint8_t val) : value(val) {}
	ModRM(const ModRM& rhs) { value = rhs.value; }

	//-------------------------------------------------------------------------
	// Overloaded Operators

	const ModRM& operator=(uint8_t val) { value = val; return *this; }

	//-------------------------------------------------------------------------
	// Member Functions

	// GetEffectiveAddress
	//
	// Gets the effective address (displacement) associated with the ModR/M byte;
	// width of the target is required for register-direct addressing mode
	template <typename _width>
	inline uintptr_t GetEffectiveAddress(ContextRecord& context)
		{ return GetEffectiveAddress(sizeof(_width), *this, context); }

	//-------------------------------------------------------------------------
	// Fields

	// The ModR/M byte defined by this class
	union {

		struct	{ uint8_t rm:3; uint8_t reg:3; uint8_t mod:2; };
		uint8_t	value;
	};

private:

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// sib_t
	//
	// Union that defines the individual values within a SIB Byte
	union sib_t {

		explicit sib_t(uint8_t val) : value(val) {}
		struct { uint8_t base:3; uint8_t index:3; uint8_t scale:2; };
		uint8_t	value;
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	static uintptr_t GetEffectiveAddress(uint8_t size, ModRM& modrm, ContextRecord& context);
	static uintptr_t GetScaledEffectiveAddress(ModRM& modrm, ContextRecord& context);
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MODRM_H_
