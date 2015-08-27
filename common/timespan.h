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

#ifndef __TIMESPAN_H_
#define __TIMESPAN_H_
#pragma once

#include <stdint.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TimeSpan
//
// TimeSpan represents a time interval

class timespan final
{
public:

	// Instance Constructors
	//
	timespan(uint64_t ticks);

	// Copy Constructor
	//
	timespan(const timespan&)=default;

	// Destructor
	//
	~timespan()=default;

	// assignment operator
	//
	timespan& operator=(const timespan& rhs);

	// uint64_t conversion operator
	//
	operator uint64_t() const;

	// addition operator
	//
	timespan operator+(const timespan& rhs) const;

	// subtraction operator
	//
	timespan operator-(const timespan& rhs) const;

	// addition assignment operator
	//
	timespan& operator+=(const timespan& rhs);
	
	// subtraction assignment operator
	//
	timespan& operator-=(const timespan& rhs);

	// equality operator
	//
	bool operator==(const timespan& rhs) const;

	// inequality operator
	//
	bool operator!=(const timespan& rhs) const;

	// greater than operator
	//
	bool operator>(const timespan& rhs) const;

	// less than operator
	//
	bool operator<(const timespan& rhs) const;

	// greater than or equal to operator
	//
	bool operator>=(const timespan& rhs) const;

	// less than or equal to operator
	//
	bool operator <=(const timespan& rhs) const;

	//-------------------------------------------------------------------------
	// Fields

	// max
	//
	// Represents the maximum allowable timespan
	static const timespan max;

	// zero
	//
	// Represents a zero-length timespan
	static const timespan zero;

	//-------------------------------------------------------------------------
	// Member Functions

	// days
	//
	// Gets the number of days represented by the timespan
	uint64_t days(void) const;

	// days (static)
	//
	// Constructs a timespan from the specified number of days
	static timespan days(uint32_t days);

	// hours
	//
	// Gets the number of hours represented by the timespan
	uint64_t hours(void) const;

	// hours (static)
	//
	// Constructs a timespan from the specified number of hours
	static timespan hours(uint32_t hours);

	// microseconds
	//
	// Gets the number of microseconds represented by the timespan
	uint64_t microseconds(void) const;

	// microseconds (static)
	//
	// Constructs a timespan from the specified number of microseconds
	static timespan microseconds(uint32_t microseconds);

	// milliseconds
	//
	// Gets the number of milliseconds represented by the timespan
	uint64_t milliseconds(void) const;

	// milliseconds (static)
	//
	// Constructs a timespan from the specified number of milliseconds
	static timespan milliseconds(uint32_t milliseconds);

	// minutes
	//
	// Gets the number of minutes represented by the timespan
	uint64_t minutes(void) const;

	// minutes (static)
	//
	// Constructs a timespan from the specified number of minutes
	static timespan minutes(uint32_t minutes);

	// seconds
	//
	// Gets the number of seconds represented by the timespan
	uint64_t seconds(void) const;

	// seconds (static)
	//
	// Constructs a timespan from the specified number of seconds
	static timespan seconds(uint32_t seconds);

private:

	//-------------------------------------------------------------------------
	// Constants

	// MICROSECOND
	//
	// Number of ticks in a microsecond
	static const uint64_t MICROSECOND{ 10ui64 };

	// MILLISECOND
	//
	// Number of ticks in a millisecond
	static const uint64_t MILLISECOND{ 1000ui64 * MICROSECOND };

	// SECOND
	//
	// Number of ticks in a second
	static const uint64_t SECOND{ 1000ui64 * MILLISECOND };

	// MINUTE
	//
	// Number of ticks in a minute
	static const uint64_t MINUTE{ 60ui64 * SECOND };

	// HOUR
	//
	// Number of ticks in an hour
	static const uint64_t HOUR{ 60ui64 * MINUTE };

	// DAY
	//
	// Number of ticks in a day
	static const uint64_t DAY{ 24ui64 * HOUR };

	//-------------------------------------------------------------------------
	// Member Variables

	uint64_t			m_ticks;		// Time interval (100ns increments)
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TIMESPAN_H_
