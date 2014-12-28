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

#ifndef __COMMANDLINE_H_
#define __COMMANDLINE_H_
#pragma once

#include <map>
#include <memory>
#include <vector>
#include "generic_text.h"
#include "Exception.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// CommandLine
//
// Command line processor class.  Not intended to work like getopt() or any
// fancy processors, this just breaks the command line up into arguments and
// switched arguments and allows iteration/examination of them.  Validation
// of the arguments themselves is not provided
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
class CommandLineArguments;					// Forward decl of private type
class CommandLineSwitches;					// Forward decl of private type
public:

	// Constructors
	CommandLine(int argc, tchar_t** argv);
	CommandLine(const tchar_t* commandline);

	//-------------------------------------------------------------------------
	// Properties

	// Arguments
	//
	// Gets a reference to the contained CommandLineArguments instance
	__declspec(property(get=getArguments)) const CommandLineArguments& Arguments;
	const CommandLineArguments& getArguments(void) const { return m_args; }

	// Switches
	//
	// Gets a reference to the contained CommandLineSwitches instance
	__declspec(property(get=getSwitches)) const CommandLineSwitches& Switches;
	const CommandLineSwitches& getSwitches(void) const { return m_switches; }

private:

	CommandLine(const CommandLine&)=delete;
	CommandLine& operator=(const CommandLine&)=delete;

	// Private Constructor
	CommandLine(const std::vector<std::tstring>& rawargs);

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// CommandLineArguments
	//
	// Collection type for the unswitched arguments
	class CommandLineArguments
	{
	public:

		// Constructor
		explicit CommandLineArguments(const std::vector<std::tstring>& rawargs);

		// Subscript operator
		std::tstring operator[](int index) const { return Get(index); }

		// Get
		//
		// Retrieves the unswitched argument at the specified index
		std::tstring Get(int index) const;

		// Count
		//
		// Gets the number of unswitched arguments
		__declspec(property(get=getCount)) int Count;
		int getCount(void) const { return static_cast<int>(m_args.size()); }

	private:

		CommandLineArguments(const CommandLineArguments&)=delete;
		CommandLineArguments& operator=(const CommandLineArguments&)=delete;

		// m_args
		//
		// Contained collection of unswitched argument strings
		std::vector<std::tstring> m_args;
	};

	// CommandLineSwitches
	//
	// Collection type for switched arguments
	class CommandLineSwitches
	{
	public:

		// Constructor
		explicit CommandLineSwitches(const std::vector<std::tstring>& rawargs);

		// Contains
		//
		// Determines if the collection contains at least one of the switches
		bool Contains(const std::tstring& key) const;

		// GetValue
		//
		// Gets the first value associated with a switch key
		std::tstring GetValue(const std::tstring& key) const;

		// GetValues
		//
		// Gets all values associated with a switch key
		std::vector<std::tstring> GetValues(const std::tstring key) const;

	private:

		CommandLineSwitches(const CommandLineSwitches&)=delete;
		CommandLineSwitches& operator=(const CommandLineSwitches&)=delete;

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
	
		// m_switches
		//
		// Collection of switched arguments and optional values
		std::multimap<std::tstring, std::tstring, SwitchCompare> m_switches;
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	// MakeVector
	//
	// Converts the raw argument data into a temporary vector<>
	static std::vector<std::tstring> MakeVector(const tchar_t* commandline);
	static std::vector<std::tstring> MakeVector(int argc, tchar_t** argv);

	//-------------------------------------------------------------------------
	// Member Variables

	CommandLineArguments		m_args;				// Unswitched arguments
	std::tstring				m_executable;		// Executable name (argv[0])
	CommandLineSwitches			m_switches;			// Switched arguments
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __COMMANDLINE_H_
