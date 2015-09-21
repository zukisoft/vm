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

#ifndef __EXECUTABLEFORMAT_H_
#define __EXECUTABLEFORMAT_H_
#pragma once

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ExecutableFormat Enumeration
//
// Defines the file format of an executable image

enum class ExecutableFormat
{
	ELF			= 0,			// ELF executable format
	//AOut		= 1,			// A.OUT executable format (todo: future - see include/uapi/linux/a.out.h)
	// note: there are multiple A.OUT formats - OMAGIC, NMAGIC, ZMAGIC, QMAGIC, SMAGIC and so on
	// FatELF	= 2,			// TODO: Don't forget to add support for these
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EXECUTABLEFORMAT_H_