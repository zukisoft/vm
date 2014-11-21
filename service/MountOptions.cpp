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
#include "MountOptions.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// MountOptions::MakeVector (private, static)
//
// Converts a mount data argument string into a vector of individual strings
//
// Arguments:
//
//	data		- String passed into mount(2) to be converted
//	datalen		- Maximum length of the available data

std::vector<std::tstring> MountOptions::MakeVector(const void* data, size_t datalen)
{
	std::vector<std::tstring>	args;			// vector<> of argument strings

	// null is acceptable as the data argument value here, just skip it
	if(data) {

		// Copy the data into a local buffer that can be trashed by strtok()
		size_t length = min(_tcslen(reinterpret_cast<const tchar_t*>(data)), datalen) + 1;
		std::vector<tchar_t> buffer(length);
		memcpy(buffer.data(), data, buffer.size() * sizeof(tchar_t));

		// Use good old strtok() to break up the copy of the string by commas
		tchar_t separators[] = _T(",");
		tchar_t* context = nullptr;

		const tchar_t* next = _tcstok_s(buffer.data(), separators, &context);
		while(next) {

			args.push_back(next);
			next = _tcstok_s(nullptr, separators, &context);
		}
	}

	return args;								// Return generated vector<>
}

//-----------------------------------------------------------------------------
// MountOptions::MountArguments Constructor
//
// Arguments:
//
//	rawargs		- Vector<> of all raw mount argument strings

MountOptions::MountArguments::MountArguments(const std::vector<std::tstring>& rawargs)
{
	// Iterate over all the raw arguments to find and process the switched ones
	for(const auto& arg : rawargs) {

		// Ignore blank arguments
		if(arg.length() > 0) {

			// Insert the switch into the collection, with or with the optional value after a colon
			size_t equalsign = arg.find(_T('='));
			if(equalsign == std::tstring::npos) { m_col.insert(std::make_pair(trim(arg), std::tstring())); }
			else { m_col.insert(std::make_pair(trim(arg.substr(0, equalsign)), trim(arg.substr(equalsign + 1)))); }
		}
	}
}

//-----------------------------------------------------------------------------
// MountOptions::MountArguments::Contains
//
// Determines if the collection contains at least one of the specified keys
//
// Arguments:
//
//	key		- Switch name/key to check in the collection

bool MountOptions::MountArguments::Contains(const std::tstring& key) const
{
	// Use the count of the specified key in the collection to determine this
	return (m_col.count(key) > 0);
}

//-----------------------------------------------------------------------------
// MountOptions::MountArguments::GetValue
//
// Retrieves the first value associated with the specified key
//
// Arguments:
//
//	key		- Switch name/key to retrieve a single value for

std::tstring MountOptions::MountArguments::GetValue(const std::tstring& key) const
{
	// Use find to locate the first element with the specified key
	const auto& iterator = m_col.find(key);
	return (iterator == m_col.cend()) ? std::tstring() : iterator->second;
}

//-----------------------------------------------------------------------------
// MountOptions::MountArguments::GetValues
//
// Retrieves all the values associated with the specified key
//
// Arguments:
//
//	key		- Switch name/key to retrieve all values for

std::vector<std::tstring> MountOptions::MountArguments::GetValues(const std::tstring key) const
{
	std::vector<std::tstring> values;			// vector<> of values to return

	// Use equal_range to retrieve all the elements with the specified key
	const auto& range = m_col.equal_range(key);
	for(auto iterator = range.first; iterator != range.second; iterator++) values.push_back(iterator->second);

	return values;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
