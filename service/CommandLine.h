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

#ifndef __COMMANDLINE_H_
#define __COMMANDLINE_H_
#pragma once

#include <map>
#include <vector>
#include "char_t.h"
#include "tstring.h"
#include "Exception.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// CommandLine
//
// Command line processor class.  Not intended to work like getopt() or any
// fancy processors, this just breaks the command line up into arguments and
// switched arguments and allows iteration/examination of them.  Validation
// of the arguments themselves is not provided.
//
// Switched arguments must start with a hypen or a slash, and can optionally be
// associated with a value by using a colon as the delimiter:
//
//	-myswitch:myswitchvalue
//
// Unswitched arguments are collected in the order in which they appear in
// the command line and can only be accessed via index or an iteration

class CommandLine
{
public:

	// Constructors
	CommandLine(int argc, tchar_t** argv);
	CommandLine(const tchar_t* commandline);

	//-------------------------------------------------------------------------
	// Member Functions

	//-------------------------------------------------------------------------
	// Properties

private:

	CommandLine(const CommandLine&)=delete;
	CommandLine& operator=(const CommandLine&)=delete;

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// ArgumentVector
	//
	// vector<> of unswitched command line argument strings
	using ArgumentVector = std::vector<std::tstring>;

	// SwitchCompare
	//
	// Case-insensitive key comparison for the switch collection
	struct SwitchCompare 
	{ 
		bool operator() (const std::tstring& lhs, const std::tstring& rhs) const 
		{
			return _tcsicmp(lhs.c_str(), rhs.c_str()) < 0;
		}
	};

	// SwitchMultiMap
	//
	// multimap<> of switched command line argument strings and values
	using SwitchMultiMap = std::multimap<std::tstring, std::tstring, SwitchCompare>;

	// make public!
public:
	__declspec(property(get=getArguments)) ArgumentVector Arguments;
	ArgumentVector getArguments(void) //-> ArgumentVector
	{
		return m_args;
	}

	// need custom iterator for keys!
	//auto getSwitches(void) -> blah

private:

	//-------------------------------------------------------------------------
	// Member Variables

	ArgumentVector			m_args;				// Standard argument strings
	std::tstring			m_executable;		// Executable name (argv[0])
	SwitchMultiMap			m_switches;			// Switched argument strings
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __COMMANDLINE_H_
