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

#ifndef __GENERIC_TEXT_H_
#define __GENERIC_TEXT_H_
#pragma once

#include <string>
#include <type_traits>
#include <vector>
#include <tchar.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Generic Text Extensions

// char_t
//
// Typedef for an ANSI character
using char_t = char;

// tchar_t
//
// Typedef for a generic text character
using tchar_t = std::conditional<sizeof(TCHAR) == sizeof(wchar_t), wchar_t, char>::type;

namespace std { 
	
	// std::tstring
	//
	// Typedef for a generic text std::[w]string
	using tstring = conditional<sizeof(TCHAR) == sizeof(wchar_t), wstring, string>::type;

	// std::to_string (conversion)
	//
	inline string to_string(const wchar_t* psz)
	{
		if(psz == nullptr) return string();

		// Create an std::vector big enough to hold the converted string data and convert it
		vector<char_t> convert(WideCharToMultiByte(CP_THREAD_ACP, 0, psz, -1, nullptr, 0, nullptr, nullptr));
		WideCharToMultiByte(CP_THREAD_ACP, 0, psz, -1, convert.data(), static_cast<int>(convert.size()), nullptr, nullptr);

		// Construct an std::string around the converted character data
		return string(convert.data(), convert.size());
	}

	// std::to_string overloads
	//
	inline string to_string(const char_t* psz) { return string(psz); }
	inline string to_string(const wstring& str) { return to_string(str.c_str()); }

	// std::to_wstring (conversion)
	//
	inline wstring to_wstring(const char_t* psz)
	{
		if(psz == nullptr) return wstring();

		// Create an std::vector big enough to hold the converted string data and convert it
		vector<wchar_t> convert(MultiByteToWideChar(CP_THREAD_ACP, 0, psz, -1, nullptr, 0));
		MultiByteToWideChar(CP_THREAD_ACP, 0, psz, -1, convert.data(), static_cast<int>(convert.size()));

		// Construct an std::wstring around the converted character data
		return wstring(convert.data(), convert.size());
	}

	// std::to_wstring overloads
	//
	inline wstring to_wstring(const wchar_t* psz) { return wstring(psz); }
	inline wstring to_wstring(const string& str) { return to_wstring(str.c_str()); }

	// std::to_tstring
	//
	// Generic text wrapper around std::to_[w]string
	template <typename _type> inline tstring to_tstring(_type value)
	{
#ifdef _UNICODE
		return to_wstring(value);
#else
		return to_string(value);
#endif
	}

} // namespace std

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __GENERIC_TEXT_H_
