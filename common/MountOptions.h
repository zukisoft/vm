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

#ifndef __MOUNTOPTIONS_H_
#define __MOUNTOPTIONS_H_
#pragma once

#include <map>
#include <vector>
#include <linux/fs.h>

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// MountOptions
//
// Linux mounting options processor class.  This accepts the general mount flags
// and optional data arguments from mount(2) and parses them out.  Optional data
// arguments are collected as strings that can be retrieved and processed via the
// .Arguments collection

class MountOptions
{
	// Forward Declarations
	//
	class MountArguments;

public:

	// Instance Constructors
	//
	MountOptions(uint32_t flags);
	MountOptions(const char_t* options);
	MountOptions(uint32_t flags, const char_t* options);
	MountOptions(uint32_t flags, const void* data, size_t datalen);

	// Destructor
	//
	~MountOptions()=default;

	// Array subscript operators
	//
	uint32_t operator[](uint32_t flag) const;
	std::string operator[](const std::string& key) const;

	//-------------------------------------------------------------------------
	// Properties

	// Arguments
	//
	// Accesses the contained non-standard arguments collection
	__declspec(property(get=getArguments)) const MountArguments& Arguments;
	const MountArguments& getArguments(void) const;

	// Flags
	//
	// Gets the standard mounting flags
	__declspec(property(get=getFlags)) uint32_t Flags;
	uint32_t getFlags(void) const;

private:

	MountOptions(const MountOptions&)=delete;
	MountOptions& operator=(const MountOptions&)=delete;

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// MountOptions::MountArguments
	//
	// Collection type for non-standard mounting option strings
	class MountArguments
	{
	friend class MountOptions;
	public:

		// Destructor
		//
		~MountArguments()=default;

		// Array subscript operator
		//
		std::string operator[](const std::string& key) const;

		//---------------------------------------------------------------------
		// Member Functions

		// Contains
		//
		// Determines if the collection contains at least one of the switches
		bool Contains(const std::string& key) const;

		// GetValue
		//
		// Gets the first value associated with a switch key
		std::string GetValue(const std::string& key) const;

		// GetValues
		//
		// Gets all values associated with a switch key
		std::vector<std::string> GetValues(const std::string key) const;

	private:

		MountArguments(const MountArguments&)=delete;
		MountArguments& operator=(const MountArguments&)=delete;

		//---------------------------------------------------------------------
		// Private Type Declarations

		// compare_t
		//
		// Case-insensitive key comparison for the argument collection
		struct compare_t 
		{ 
			bool operator() (const std::string& lhs, const std::string& rhs) const 
			{ 
				return _stricmp(lhs.c_str(), rhs.c_str()) < 0; 
			}
		};

		// collection_t
		//
		// Case-insensitive multimap<string, string> to hold the arguments
		using collection_t = std::multimap<std::string, std::string, compare_t>;
	
		// Instance Constructor
		//
		MountArguments()=default;

		//---------------------------------------------------------------------
		// Member Variables

		collection_t			m_col;			// Arguments collection
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ParseToken (static)
	//
	// Parses a single options string token into either a flag or a key/value pair
	static void ParseToken(std::string&& token, uint32_t& flags, MountArguments& arguments);

	//-------------------------------------------------------------------------
	// Member Variables

	uint32_t					m_flags;			// Standard mounting flags
	MountArguments				m_arguments;		// Non-standard arguments
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MOUNTOPTIONS_H_
