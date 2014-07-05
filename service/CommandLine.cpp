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

CommandLine::CommandLine(int argc, tchar_t** argv)
{
	// There needs to be at least one argument, which is the executable name
	if(argc == 0) throw Exception(E_INVALIDARG);
	if(argv[0] == nullptr) throw Exception(E_POINTER);
	m_executable = argv[0];

	// Iterate over the remaining command line arguments
	for(int index = 1; index < argc; index++) {

		// NULL pointer or zero-length string -- skip it
		if((argv[index] == nullptr) || (argv[index][0] == 0)) continue;

		// Switched arguments start with a hypen or a slash and optionally 
		// contain a string value after a colon [-switch[:value]]
		if((argv[index][0] == _T('-')) || (argv[index][0] == _T('/'))) {

			std::tstring arg = &argv[index][1];			// Convert into a tstring
			size_t colon = arg.find(_T(':'));			// Find the colon

			// Insert the switch into the collection
			if(colon == std::tstring::npos) { m_switches.insert(std::make_pair(arg, std::tstring())); }
			else { m_switches.insert(std::make_pair(arg.substr(0, colon), arg.substr(colon + 1))); }
		}

		// Any non-switch argument is just pushed in order to the vector<>
		else m_args.push_back(argv[index]);
	}
}

//-----------------------------------------------------------------------------
// CommandLine Constructor
//
// Arguments:
//
//	commandline		- Unprocessed command line string

CommandLine::CommandLine(const tchar_t* commandline)
{
	std::vector<tchar_t>	buffer(_MAX_PATH);	// Executable name buffer
	size_t					length = 0;			// Executable name length
	int						argc = 0;			// Number of additional arguments

	// GetModuleFileName() is horrible and you have to call it over and over
	// with progressively larger buffers until it actually succeeds ...
	do { 

		length = GetModuleFileName(nullptr, buffer.data(), buffer.size());
		if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) buffer.resize(buffer.size() << 1);

	} while(GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	// Set the contained executable name using a move rather than a copy
	m_executable = std::tstring(buffer.data(), length);

	// If no command line was specified, there is nothing to do
	if((commandline == nullptr) || (commandline[0] == 0)) return;
	
	// Convert the string into a Unicode command line argument array (no ANSI equivalent)
	wchar_t** wargv = CommandLineToArgvW(std::make_wstring(commandline).c_str(), &argc);

	// Iterate over the generated array of command line arguments
	for(int index = 0; index < argc; index++) {

		// NULL pointer or zero-length string -- skip it
		if((wargv[index] == nullptr) || (wargv[index][0] == 0)) continue;

		// Switched arguments start with a hypen or a slash and optionally 
		// contain a string value after a colon [-switch[:value]]
		if((wargv[index][0] == L'-') || (wargv[index][0] == L'/')) {

			std::tstring arg = std::make_tstring(&wargv[index][1]);	// Convert into a tstring
			size_t colon = arg.find(_T(':'));						// Find the colon

			// Insert the switch into the collection
			if(colon == std::tstring::npos) { m_switches.insert(std::make_pair(arg, std::tstring())); }
			else { m_switches.insert(std::make_pair(arg.substr(0, colon), arg.substr(colon + 1))); }
		}

		// Any non-switch argument is just pushed in order to the vector<>
		else m_args.push_back(std::make_tstring(wargv[index]));
	}

	LocalFree(wargv);						// Release buffer allocated by API call
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
