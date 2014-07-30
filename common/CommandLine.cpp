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

#include "stdafx.h"
#include "CommandLine.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// CommandLine Constructor
//
// Arguments:
//
//	argc			- Number of command line arguments
//	argv			- Array of command line argument strings

CommandLine::CommandLine(int argc, tchar_t** argv) : CommandLine(MakeVector(argc, argv))
{
	// The executable name is expected to be set in argv[0] when using this constructor
	if((argc == 0) || (argv[0] == nullptr)) throw Exception(E_INVALIDARG);
	m_executable = argv[0];
}

//-----------------------------------------------------------------------------
// CommandLine Constructor
//
// Arguments:
//
//	commandline		- Unprocessed command line string

CommandLine::CommandLine(const tchar_t* commandline) : CommandLine(MakeVector(commandline))
{
	std::vector<tchar_t>	buffer(_MAX_PATH);	// Executable name buffer
	size_t					length = 0;			// Executable name length

	do { 
		// GetModuleFileName() is horrible and doesn't have a way to find out how much
		// space you actually need; call it repeatedly doubling the buffer each time
		length = GetModuleFileName(nullptr, buffer.data(), buffer.size());
		if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) buffer.resize(buffer.size() << 1);

	} while(GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	// Set the contained executable name using a move rather than a copy
	m_executable = std::tstring(buffer.data(), length);
}

//-----------------------------------------------------------------------------
// CommandLine Constructor (private)
//
// Arguments:
//
//	rawargs		- All raw arguments as a vector<tstring> collection

CommandLine::CommandLine(const std::vector<std::tstring>& rawargs) : m_args(rawargs), m_switches(rawargs)
{
	// nothing interesting to do here?
}

//-----------------------------------------------------------------------------
// CommandLine::MakeVector (private, static)
//
// Converts a command line argument string into a vector of individual strings
//
// Arguments:
//
//	commandline		- Command line string to be converted

std::vector<std::tstring> CommandLine::MakeVector(const tchar_t* commandline)
{
	int							argc = 0;		// Number of argument strings
	std::vector<std::tstring>	args;			// vector<> of argument strings

	// No command line was specified, return the empty vector<> instance
	if((commandline == nullptr) || (commandline[0] == 0)) return args;

	// Convert the command line into an argc/argv array; no generic text version of this
	wchar_t** wargv = CommandLineToArgvW(std::to_wstring(commandline).c_str(), &argc);

	// Convert each returned command line argument into a tstring for the vector<>
	for(int index = 0; index < argc; index++) args.push_back(std::to_tstring(wargv[index]));

	LocalFree(wargv);							// Release allocated string array
	return args;								// Return generated vector<>
}

//-----------------------------------------------------------------------------
// CommandLine::MakeVector (private, static)
//
// Converts an argc/argv argument array into a vector of tstrings.  The first
// argument (argv[0]) is expected to be the executable name and is ignored
//
// Arguments:
//
//	argc		- Number of command line arguments
//	argv		- Array of command line argument strings

std::vector<std::tstring> CommandLine::MakeVector(int argc, tchar_t** argv)
{
	std::vector<std::tstring> args;			// vector<> of argument strings

	// Skip the first argument, which will be the executable/module name string,
	// otherwise this is a simple iteration over the array to convert it
	for(int index = 1; index < argc; index++) args.push_back(argv[index]);

	return args;							// Return generated vector<>
}

//-----------------------------------------------------------------------------
// CommandLine::CommandLineArguments Constructor
//
// Arguments:
//
//	rawargs		- Vector<> of all raw command line argument strings

CommandLine::CommandLineArguments::CommandLineArguments(const std::vector<std::tstring>& rawargs)
{
	// Iterate over all the raw arguments and add just the ones that aren't switches
	for(const auto& arg : rawargs)
		if((arg.length() > 0) && (arg[0] != _T('-')) && (arg[0] != _T('/'))) m_args.push_back(arg);
}

//-----------------------------------------------------------------------------
// CommandLine::CommandLineArguments::Get
//
// Retrieves the argument at the specified index
//
// Arguments:
//
//	index		- Unswitched argument index

std::tstring CommandLine::CommandLineArguments::Get(int index) const
{
	// Check the index against the vector<> size and return the string
	if(static_cast<size_t>(index + 1) > m_args.size()) return std::tstring();
	return m_args[index];
}

//-----------------------------------------------------------------------------
// CommandLine::CommandLineSwitches Constructor
//
// Arguments:
//
//	rawargs		- Vector<> of all raw command line argument strings

CommandLine::CommandLineSwitches::CommandLineSwitches(const std::vector<std::tstring>& rawargs)
{
	// Iterate over all the raw arguments to find and process the switched ones
	for(const auto& arg : rawargs) {

		// Switches all start with a hyphen or a forward slash 
		if((arg.length() > 0) && ((arg[0] == _T('-')) || (arg[0] == _T('/')))) {

			// Insert the switch into the collection, with or with the optional value after a colon

			size_t colon = arg.find(_T(':'));
			if(colon == std::tstring::npos) { m_switches.insert(std::make_pair(arg.substr(1), std::tstring())); }
			else { m_switches.insert(std::make_pair(arg.substr(1, colon - 1), arg.substr(colon + 1))); }
		}
	}
}

//-----------------------------------------------------------------------------
// CommandLine::CommandLineSwitches::Contains
//
// Determines if the collection contains at least one of the specified keys
//
// Arguments:
//
//	key		- Switch name/key to check in the collection

bool CommandLine::CommandLineSwitches::Contains(const std::tstring& key) const
{
	// Use the count of the specified key in the collection to determine this
	return (m_switches.count(key) > 0);
}

//-----------------------------------------------------------------------------
// CommandLine::CommandLineSwitches::GetValue
//
// Retrieves the first value associated with the specified key
//
// Arguments:
//
//	key		- Switch name/key to retrieve a single value for

std::tstring CommandLine::CommandLineSwitches::GetValue(const std::tstring& key) const
{
	// Use find to locate the first element with the specified key
	const auto& iterator = m_switches.find(key);
	return (iterator == m_switches.cend()) ? std::tstring() : iterator->second;
}

//-----------------------------------------------------------------------------
// CommandLine::CommandLineSwitches::GetValues
//
// Retrieves all the values associated with the specified key
//
// Arguments:
//
//	key		- Switch name/key to retrieve all values for

std::vector<std::tstring> CommandLine::CommandLineSwitches::GetValues(const std::tstring key) const
{
	std::vector<std::tstring> values;			// vector<> of values to return

	// Use equal_range to retrieve all the elements with the specified key
	const auto& range = m_switches.equal_range(key);
	for(auto iterator = range.first; iterator != range.second; iterator++) values.push_back(iterator->second);

	return values;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
