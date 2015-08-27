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

#ifndef __CONVERT_H_
#define __CONVERT_H_
#pragma once

#include <type_traits>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// convert<> (integral type)
//
// Parameters:
//
//	_to		- Destination data type
//	_from	- Source data type value

template<typename _to, typename _from> 
typename std::enable_if<std::is_integral<_from>::value, _to>::type convert(_from rhs);

//-----------------------------------------------------------------------------
// convert<> (non-integral type)
//
// Parameters:
//
//	_to		- Destination data type
//	_from	- Source data type const reference

template<typename _to, typename _from> 
typename std::enable_if<!std::is_integral<_from>::value, _to>::type convert(const _from& rhs);

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __CONVERT_H_
