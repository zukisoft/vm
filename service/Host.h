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

#ifndef __HOST_H_
#define __HOST_H_
#pragma once

#include <memory>
#include "Exception.h"
#include "Win32Exception.h"

#pragma warning(push, 4)			

class Host
{
public:

	Host(const PROCESS_INFORMATION& procinfo);
	~Host();

	static std::unique_ptr<Host> Create(const tchar_t* binarypath, const tchar_t* commandline);

private:

	Host(const Host&)=delete;
	Host& operator=(const Host&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	PROCESS_INFORMATION		m_procinfo;		// Process information
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOST_H_
