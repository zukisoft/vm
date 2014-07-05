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

#ifndef __TSTRING_H_
#define __TSTRING_H_
#pragma once

#include <string>
#include <vector>

namespace std {

#ifdef _UNICODE
	typedef wstring		tstring;
#else
	typedef string		tstring;
#endif

	// make_string (ANSI)
	//
	// Converts a null terminated C-style string into an std::string
	inline string make_string(const char_t* psz) { return string(psz); }

	// make_string (Unicode)
	//
	// Converts a null terminated C-style string into an std::string
	inline string make_string(const wchar_t* psz)
	{
		if(psz == nullptr) return string();

		// Create an std::vector big enough to hold the converted string data and convert it
		vector<char_t> convert(WideCharToMultiByte(CP_THREAD_ACP, 0, psz, -1, nullptr, 0, nullptr, nullptr));
		WideCharToMultiByte(CP_THREAD_ACP, 0, psz, -1, convert.data(), convert.size(), nullptr, nullptr);

		// Construct an std::string around the converted character data
		return string(convert.data(), convert.size());
	}

	// make_wstring (ANSI)
	//
	// Converts a null terminated C-style string into an std::wstring
	inline wstring make_wstring(const char_t* psz)
	{
		if(psz == nullptr) return wstring();

		// Create an std::vector big enough to hold the converted string data and convert it
		vector<wchar_t> convert(MultiByteToWideChar(CP_THREAD_ACP, 0, psz, -1, nullptr, 0));
		MultiByteToWideChar(CP_THREAD_ACP, 0, psz, -1, convert.data(), convert.size());

		// Construct an std::wstring around the converted character data
		return wstring(convert.data(), convert.size());
	}

	// make_wstring (Unicode)
	//
	// Converts a null terminated C-style string into an std::wstring
	inline wstring make_wstring(const wchar_t* psz) { return wstring(psz); }
	
	// make_tstring (ANSI)
	//
	// Converts a null terminate C-style string into an std::tstring
	inline tstring make_tstring(const char_t* psz)
	{
#ifdef _UNICODE
		return make_wstring(psz);
#else
		return make_string(psz);
#endif
	}

	// make_tstring (Unicode)
	//
	// Converts a null terminate C-style string into an std::tstring
	inline tstring make_tstring(const wchar_t* psz)
	{
#ifdef _UNICODE
		return make_wstring(psz);
#else
		return make_string(psz);
#endif
	}
}

//-----------------------------------------------------------------------------

#endif	// __TSTRING_H_
