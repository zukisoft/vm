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

#ifndef __DATETIME_H_
#define __DATETIME_H_
#pragma once

#include <stdint.h>
#include "timespan.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// datetime
//
// Represents a date/time based on 100ns intervals since Jauary 1, 1601 (UTC)

class datetime final
{
public:

	// Instance Constructor
	//
	datetime(uint64_t ticks);

	// Copy Constructor
	//
	datetime(const datetime&)=default;

	// Destructor
	//
	~datetime()=default;

	// assignment operator
	//
	datetime& operator=(const datetime& rhs);

	// uint64_t conversion operator
	//
	operator uint64_t() const;

	// addition operator
	//
	datetime operator+(const timespan& rhs) const;

	// subtraction operator
	//
	datetime operator-(const timespan& rhs) const;

	// addition assignment operator
	//
	datetime& operator+=(const timespan& rhs);

	// subtraction assignment operator
	//
	datetime& operator-=(const timespan& rhs);

	// equality operator
	//
	bool operator==(const datetime& rhs) const;

	// inequality operator
	//
	bool operator!=(const datetime& rhs) const;

	// greater than operator
	//
	bool operator>(const datetime& rhs) const;

	// less than operator
	//
	bool operator<(const datetime& rhs) const;

	// greater than or equal to operator
	//
	bool operator>=(const datetime& rhs) const;

	// less than or equal to operator
	//
	bool operator <=(const datetime& rhs) const;

	//-------------------------------------------------------------------------
	// Fields

	// max
	//
	// Represents the maximum allowable datetime
	static const datetime max;

	// min
	//
	// Represents the minimum allowable datetime
	static const datetime min;

	//-------------------------------------------------------------------------
	// Member Functions

	// difference
	//
	// Calculates the difference between two datetimes
	timespan difference(const datetime& rhs) const;

	// now (static)
	//
	// Gets the current date/time (UTC)
	static datetime now(void);

private:

	//-------------------------------------------------------------------------
	// Member Variables

	uint64_t			m_ticks;		// 100ns units from 1/1/1601 (FILETIME)
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __DATETIME_H_
