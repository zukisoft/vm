//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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
#include "SystemCall.h"

#include "LinuxException.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SystemCall::TranslateException
//
// Translates an std::exception into a system call result code
//
// Arguments:
//
//	ex		- Exception to be translated

uapi::long_t SystemCall::TranslateException(std::exception_ptr ex)
{
	// Re-throw the exception to handle it based on the underlying type
	try { std::rethrow_exception(ex); }
	
	// LinuxException: direct result code
	catch(LinuxException& ex) { return -ex.Code; }

	// TODO: Win32Exception translation

	//// Exception: 
	//catch(LinuxException& ex) {

	//	// TODO: SOMETHING REASONABLE HERE
	//	int x = (int)ex.Code;
	//	return -x;
	//}

	// Anything else:
	// TODO: SOMETHING REASONABLE HERE
	catch(...) { return -LINUX_EFAULT; }

	return -1;			// <--- should be impossible to reach, shuts up the compiler
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
