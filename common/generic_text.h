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

#ifndef __GENERIC_TEXT_H_
#define __GENERIC_TEXT_H_
#pragma once

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <memory>
#include <string>
#include <type_traits>
#include <tchar.h>
#include <Windows.h>

#pragma warning(push, 4)

using rpc_char_t = unsigned char;
using rpc_wchar_t = unsigned short;
using rpc_tchar_t = std::conditional<sizeof(TCHAR) == sizeof(wchar_t), rpc_wchar_t, rpc_char_t>::type;

//-----------------------------------------------------------------------------
// Generic Text Extensions
//
// TODO: This is more than generic text now, and the conversions can take 
// advantage of the convert<> template instead

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
	inline string to_string(const wchar_t* psz, int cch)
	{
		if(psz == nullptr) return string();

		// Create a buffer big enough to hold the converted string data and convert it
		int buffercch = WideCharToMultiByte(CP_UTF8, 0, psz, cch, nullptr, 0, nullptr, nullptr);
		auto buffer = std::make_unique<char_t[]>(buffercch);
		WideCharToMultiByte(CP_UTF8, 0, psz, cch, buffer.get(), buffercch, nullptr, nullptr);

		// Construct an std::string around the converted character data
		return string(buffer.get(), buffercch);
	}

	// std::to_string overloads
	//
	inline string to_string(const wchar_t* psz) { return to_string(psz, -1); }
	inline string to_string(const char_t* psz, int cch) { return string(psz, cch); }
	inline string to_string(const char_t* psz) { return string(psz); }
	inline string to_string(const wstring& str) { return to_string(str.data(), static_cast<int>(str.size())); }

	// std::to_wstring (conversion)
	//
	inline wstring to_wstring(const char_t* psz, int cch)
	{
		if(psz == nullptr) return wstring();

		// Create a buffer big enough to hold the converted string data and convert it
		int buffercch = MultiByteToWideChar(CP_UTF8, 0, psz, cch, nullptr, 0);
		auto buffer = std::make_unique<wchar_t[]>(buffercch);
		MultiByteToWideChar(CP_UTF8, 0, psz, cch, buffer.get(), buffercch);

		// Construct an std::wstring around the converted character data
		return wstring(buffer.get(), buffercch);
	}

	// std::to_wstring overloads
	//
	inline wstring to_wstring(const char_t* psz) { return to_wstring(psz, -1); }
	inline wstring to_wstring(const wchar_t* psz, int cch) { return wstring(psz, cch); }
	inline wstring to_wstring(const wchar_t* psz) { return wstring(psz); }
	inline wstring to_wstring(const string& str) { return to_wstring(str.data(), static_cast<int>(str.size())); }

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

	// std::to_tstring
	//
	// Generic text wrapper around std::to_[w]string
	template <typename _type> inline tstring to_tstring(_type value, int cch)
	{
#ifdef _UNICODE
		return to_wstring(value, cch);
#else
		return to_string(value, cch);
#endif
	}

	// std::ltrim
	//
	// Performs a left trim of a string
	inline string ltrim(const string& str)
	{
		auto left = find_if_not(str.begin(), str.end(), isspace);
		return string(left, str.end());
	}

	inline wstring ltrim(const wstring& str)
	{
		auto left = find_if_not(str.begin(), str.end(), iswspace);
		return wstring(left, str.end());
	}

	inline string ltrim(const string& str, const char_t& value)
	{
		auto left = find_if_not(str.begin(), str.end(), [&](char_t ch) { return ch == value; } );
		return string(left, str.end());
	}

	inline wstring ltrim(const wstring& str, const wchar_t& value)
	{
		auto left = find_if_not(str.begin(), str.end(), [&](wchar_t ch) { return ch == value; } );
		return wstring(left, str.end());
	}

	// std::rtrim
	//
	// Performs a right trim of a string
	inline string rtrim(const string& str)
	{
		auto right = find_if_not(str.rbegin(), str.rend(), isspace);
		return string(str.begin(), right.base());
	}

	inline wstring rtrim(const wstring& str)
	{
		auto right = find_if_not(str.rbegin(), str.rend(), iswspace);
		return wstring(str.begin(), right.base());
	}

	inline string rtrim(const string& str, const char_t& value)
	{
		auto right = find_if_not(str.rbegin(), str.rend(), [&](char_t ch) { return ch == value; } );
		return string(str.begin(), right.base());
	}

	inline wstring rtrim(const wstring& str, const wchar_t& value)
	{
		auto right = find_if_not(str.rbegin(), str.rend(), [&](wchar_t ch) { return ch == value; });
		return wstring(str.begin(), right.base());
	}

	// std::trim
	//
	// Performs a full trim of a string
	inline string trim(const string& str)
	{
		auto left = find_if_not(str.begin(), str.end(), isspace);
		auto right = find_if_not(str.rbegin(), str.rend(), isspace);
		return (right.base() <= left) ? string() : string(left, right.base());
	}

	inline wstring trim(const wstring& str)
	{
		auto left = find_if_not(str.begin(), str.end(), iswspace);
		auto right = find_if_not(str.rbegin(), str.rend(), iswspace);
		return (right.base() <= left) ? wstring() : wstring(left, right.base());
	}

	inline string trim(const string& str, const char_t& value)
	{
		auto lambda = [&](char_t ch) { return ch == value; };
		auto left = find_if_not(str.begin(), str.end(), lambda);
		auto right = find_if_not(str.rbegin(), str.rend(), lambda);
		return (right.base() <= left) ? string() : string(left, right.base());
	}

	inline wstring trim(const wstring& str, const wchar_t& value)
	{
		auto lambda = [&](wchar_t ch) { return ch == value; };
		auto left = find_if_not(str.begin(), str.end(), lambda);
		auto right = find_if_not(str.rbegin(), str.rend(), lambda);
		return (right.base() <= left) ? wstring() : wstring(left, right.base());
	}

	inline bool startswith(const string& str, const char& value)
	{
		return !str.empty() && str[0] == value;
	}

	inline bool startswith(const wstring& str, const wchar_t& value)
	{
		return !str.empty() && str[0] == value;
	}

	inline bool endswith(const string& str, const char_t& value)
	{
		return !str.empty() && str[str.size() - 1] == value;
	}

	inline bool endswith(const wstring& str, const wchar_t& value)
	{
		return !str.empty() && str[str.size() - 1] == value;
	}

} // namespace std

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __GENERIC_TEXT_H_
