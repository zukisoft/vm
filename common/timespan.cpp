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
#include "timespan.h"

#pragma warning(push, 4)

// timespan::max (static)
//
const timespan timespan::max{ INT64_MAX };

// timespan::zero (static)
//
const timespan timespan::zero{ 0ui64 };

//-----------------------------------------------------------------------------
// timespan Constructor
//
// Arguments:
//
//	ticks			- Number of 100ns intervals representing the time span

timespan::timespan(uint64_t ticks) : m_ticks(ticks)
{
	if(ticks > INT64_MAX) throw std::out_of_range("ticks");
}

//-----------------------------------------------------------------------------
// timespan assignment operator

timespan& timespan::operator=(const timespan& rhs)
{
	m_ticks = rhs.m_ticks;
	return *this;
}

//-----------------------------------------------------------------------------
// timespan uint64_t conversion operator

timespan::operator uint64_t() const
{
	return m_ticks;
}

//-----------------------------------------------------------------------------
// timespan addition operator

timespan timespan::operator+(const timespan& rhs) const
{
	return timespan(m_ticks + rhs.m_ticks);
}

//-----------------------------------------------------------------------------
// timespan subtraction operator

timespan timespan::operator-(const timespan& rhs) const
{
	return timespan((rhs.m_ticks >= m_ticks) ? 0 : m_ticks - rhs.m_ticks);
}

//-----------------------------------------------------------------------------
// timespan addition assignment operator

timespan& timespan::operator+=(const timespan& rhs)
{
	uint64_t ticks = m_ticks + rhs;
	if(ticks > INT64_MAX) throw std::out_of_range("rhs");

	m_ticks = ticks;
	return *this;
}

//-----------------------------------------------------------------------------
// timespan subtraction assignment operator

timespan& timespan::operator-=(const timespan& rhs)
{
	m_ticks = (rhs.m_ticks >= m_ticks) ? 0 : m_ticks - rhs.m_ticks;
	return *this;
}

//-----------------------------------------------------------------------------
// timespan equality operator

bool timespan::operator==(const timespan& rhs) const
{
	return m_ticks == rhs.m_ticks;
}

//-----------------------------------------------------------------------------
// timespan inequality operator

bool timespan::operator!=(const timespan& rhs) const
{
	return m_ticks != rhs.m_ticks;
}

//-----------------------------------------------------------------------------
// timespan greater than operator

bool timespan::operator>(const timespan& rhs) const
{
	return m_ticks > rhs.m_ticks;
}

//-----------------------------------------------------------------------------
// timespan less than operator

bool timespan::operator<(const timespan& rhs) const
{
	return m_ticks < rhs.m_ticks;
}

//-----------------------------------------------------------------------------
// timespan greater than or equal to operator

bool timespan::operator>=(const timespan& rhs) const
{
	return m_ticks >= rhs.m_ticks;
}

//-----------------------------------------------------------------------------
// timespan less than or equal to operator

bool timespan::operator<=(const timespan& rhs) const
{
	return m_ticks <= rhs.m_ticks;
}

//-----------------------------------------------------------------------------
// timespan::days
//
// Gets the number of days represented by the timespan
//
// Arguments:
//
//	NONE

uint64_t timespan::days(void) const
{
	return m_ticks / DAY;
}

//-----------------------------------------------------------------------------
// timespan::days (static)
//
// Constructs a new timespan representing the specified number of days
//
// Arguments:
//
//	days		- Number of days from which to create the timespan

timespan timespan::days(uint32_t days)
{
	return timespan(DAY * days);
}

//-----------------------------------------------------------------------------
// timespan::hours
//
// Gets the number of hours represented by the timespan
//
// Arguments:
//
//	NONE

uint64_t timespan::hours(void) const
{
	return m_ticks / HOUR;
}

//-----------------------------------------------------------------------------
// timespan::hours (static)
//
// Constructs a new timespan representing the specified number of hours
//
// Arguments:
//
//	hours		- Number of hours from which to create the timespan

timespan timespan::hours(uint32_t hours)
{
	return timespan(HOUR * hours);
}

//-----------------------------------------------------------------------------
// timespan::microseconds
//
// Gets the number of microseconds represented by the timespan
//
// Arguments:
//
//	NONE

uint64_t timespan::microseconds(void) const
{
	return m_ticks / MICROSECOND;
}

//-----------------------------------------------------------------------------
// timespan::microseconds (static)
//
// Constructs a new timespan representing the specified number of microseconds
//
// Arguments:
//
//	microseconds	- Number of microseconds from which to create the timespan

timespan timespan::microseconds(uint32_t microseconds)
{
	return timespan(MICROSECOND * microseconds);
}

//-----------------------------------------------------------------------------
// timespan::milliseconds
//
// Gets the number of milliseconds represented by the timespan
//
// Arguments:
//
//	NONE

uint64_t timespan::milliseconds(void) const
{
	return m_ticks / MILLISECOND;
}

//-----------------------------------------------------------------------------
// timespan::milliseconds (static)
//
// Constructs a new timespan representing the specified number of milliseconds
//
// Arguments:
//
//	milliseconds	- Number of milliseconds from which to create the timespan

timespan timespan::milliseconds(uint32_t milliseconds)
{
	return timespan(MILLISECOND * milliseconds);
}

//-----------------------------------------------------------------------------
// timespan::minutes
//
// Gets the number of minutes represented by the timespan
//
// Arguments:
//
//	NONE

uint64_t timespan::minutes(void) const
{
	return m_ticks / MINUTE;
}

//-----------------------------------------------------------------------------
// timespan::minutes (static)
//
// Constructs a new timespan representing the specified number of minutes
//
// Arguments:
//
//	minutes		- Number of minutes from which to create the timespan

timespan timespan::minutes(uint32_t minutes)
{
	return timespan(MINUTE * minutes);
}

//-----------------------------------------------------------------------------
// timespan::seconds
//
// Gets the number of seconds represented by the timespan
//
// Arguments:
//
//	NONE

uint64_t timespan::seconds(void) const
{
	return m_ticks / SECOND;
}

//-----------------------------------------------------------------------------
// timespan::seconds (static)
//
// Constructs a new timespan representing the specified number of seconds
//
// Arguments:
//
//	seconds		- Number of seconds from which to create the timespan

timespan timespan::seconds(uint32_t seconds)
{
	return timespan(SECOND * seconds);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
