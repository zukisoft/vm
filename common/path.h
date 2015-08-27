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

#ifndef __PATH_H_
#define __PATH_H_
#pragma once

#include <memory>
#include <PathCch.h>
#include "Win32Exception.h"

#pragma comment(lib, "pathcch.lib")

#pragma warning(push, 4)

// path_format
//
// Defines which path_operations specializations to apply to a path<> instance
enum class path_format
{
	posix	= 0,			// UTF-8
	windows,				// UTF-16
};

// path_operations
//
// Specializes the path behavior for a specific system format
template<path_format> struct path_operations;

// path_operations<posix>
//
// POSIX path specializations
template<> struct path_operations<path_format::posix>
{
	// pathchar_t
	//
	// Basic path character data type
	using pathchar_t = char;

	// delimiter
	//
	// Path delimiter character
	static const pathchar_t delimiter = '/';

	// append (const char*)
	//
	// Concatenates a path component to an existing string
	template<typename... _arguments>
	static void append(char* dest, size_t length, const char* next, const _arguments&... args)
	{
		*dest++ = delimiter;			// Append the delimiter character
		--length;						// Consumed one character

		// Copy and increment/decrement until the null terminator is reached
		while(*next) { *dest++ = *next++; --length; }

		// Append the next path component
		append(dest, length, args...);
	}

	// append (const wchar_t*)
	//
	// Concatenates a path component to an existing string
	template<typename... _arguments>
	static void append(char* dest, size_t length, const wchar_t* next, const _arguments&... args)
	{
		*dest++ = delimiter;			// Append the delimiter character
		--length;						// Consumed one character

		// Convert the UTF-16 string into UTF-8 for the path
		size_t cch = WideCharToMultiByte(CP_UTF8, 0, next, -1, dest, length, nullptr, nullptr);
		if(cch == 0) throw Win32Exception(GetLastError());

		// WideCharToMultiByte returns the count with a null terminator when a null-
		// terminated string is provided, account for that here
		append(dest + (cch - 1), length - (cch - 1), args...);
	}

	// append (const std::string&)
	//
	// Concatenates a path component to an existing string
	template<typename... _arguments>
	static void append(char* dest, size_t length, const std::string& next, const _arguments&... args)
	{
		*dest++ = delimiter;			// Append the delimiter character
		--length;						// Consumed one character

		// Get the length of the string data and copy it into the path
		size_t cch = next.length();
		copy(dest, length, next.data(), cch);

		// Append the next path component
		append(dest + cch, length - cch, args...);
	}

	// append (const std::wstring&)
	//
	// Concatenates a path component to an existing string
	template<typename... _arguments>
	static void append(char* dest, size_t length, const std::wstring& next, const _arguments&... args)
	{
		*dest++ = delimiter;			// Append the delimiter character
		--length;						// Consumed one character

		// Convert the UTF-16 input string into UTF-8 for the path
		size_t cch = WideCharToMultiByte(CP_UTF8, 0, next.data(), next.length(), dest, length, nullptr, nullptr);
		if(cch == 0) throw Win32Exception(GetLastError());

		// WideCharToMultiByte does not include the null terminator in the count when a
		// fixed-length input string is provided
		append(dest + cch, length - cch, args...);
	}

	// append (void)
	//
	// Final link in variadic template chain for append()
	static void append(char* dest, size_t length)
	{
		_ASSERTE(length == 1);		// should be exactly one character left
		*dest = 0;					// null terminate the string
	}

	// compare
	//
	// Compares two null-terminated strings
	static int compare(const char* left, const char* right)
	{
		if(left == right) return 0;
		else return strcmp(left, right);
	}

	// copy
	//
	// Copies a source string into a destination buffer
	static void copy(char* dest, size_t length, const char* source, size_t cch)
	{
		strncpy_s(dest, length, source, cch);
	}

	// count (const char*)
	//
	// Counts the number of characters in an input string; does not include null terminator
	template<typename... _arguments>
	static size_t count(const char* next, const _arguments&... args)
	{
		return /* delimiter */ 1 + length(next) + count(args...);
	}

	// count (const wchar_t*)
	//
	// Counts the number of characters in an input string; does not include null terminator
	template<typename... _arguments>
	static size_t count(const wchar_t* next, const _arguments&... args)
	{
		// Determine the number of UTF-8 characters required for the string
		size_t cch = WideCharToMultiByte(CP_UTF8, 0, next, -1, nullptr, 0, nullptr, nullptr);
		if(cch == 0) throw Win32Exception(GetLastError());

		// WideCharToMultiByte returns the length with null termination when -1
		// has been specified for the input length; use that for the delimiter
		return cch + count(args...);
	}

	// count (const std::string&)
	//
	// Counts the number of characters in an input string; does not include null terminator
	template<typename... _arguments>
	static size_t count(const std::string& next, const _arguments&... args)
	{
		return /* delimiter */ 1 + next.length() + count(args...);
	}

	// count (const std::wstring&)
	//
	// Counts the number of characters in an input string; does not include null terminator
	template<typename... _arguments>
	static size_t count(const std::wstring& next, const _arguments&... args)
	{
		// Determine the number of UTF-8 characters required for the string
		size_t cch = WideCharToMultiByte(CP_UTF8, 0, next.data(), next.length(), nullptr, 0, nullptr, nullptr);
		if(cch == 0) throw Win32Exception(GetLastError());

		// WideCharToMultiByte does not include the null terminator in the count
		// when a fixed-length input string was specified, so add the delimiter
		return /* delimiter */ 1 + cch + count(args...);
	}

	// count (void)
	//
	// Final link in variadic template chain for count()
	static size_t count(void)
	{
		return /* null terminator */ 1;
	}

	// dedupe_delimiters
	//
	// Removes all duplicate delimiters in a path string
	static size_t dedupe_delimiters(char* str, size_t length)
	{
		if((*str == 0) || (length == 0)) return 0;

		size_t i = 1;
		size_t j = 1;

		while(j < length - 1)
		{
			if(str[j] == delimiter && str[i - 1] == delimiter) ++j;
			else str[i++] = str[j++];
		}

		str[i] = 0;
		return i + 1;
	}

	// find_leaf
	//
	// Locates the position of the path leaf component
	static char* find_leaf(char* str, size_t length)
	{
		if((*str == 0) || (length == 0)) return str;

		// Start at the end of the string and work backwards
		char* end = str + (length - 1);
		while((end > str) && (*end != delimiter)) --end;

		return (*end == delimiter) ? ++end : end;
	}

	// iterate
	//
	// Iterates a path string to the next component
	static char* iterate(char* str)
	{
		while((*str) && (*str != delimiter)) ++str;

		if(*str) { *str = 0; return str + 1; }
		else return str;
	}

	// length
	//
	// Determines the length of a string; does not include null terminator
	static size_t length(const char* str)
	{
		return strlen(str);
	}

	// skip_root
	//
	// Skips over the root portion of a path
	static char* skip_root(char* str)
	{
		return (*str == delimiter) ? str + 1 : str;
	}

	// split
	//
	// Splits a path by inserting a null terminator in it, there must
	// be enough room to move the string right by one character
	static char* split(char* str, char* at, size_t length)
	{
		memmove(at + 1, at, &str[length - 1] - at);
		*at++ = str[length - 1] = 0;

		return at;
	}
};

// path_operations<windows>
//
// Windows path specializations
template<> struct path_operations<path_format::windows>
{
	// pathchar_t
	//
	// Basic path character data type
	using pathchar_t = wchar_t;

	// delimiter
	//
	// Path delimiter character
	static const pathchar_t delimiter = L'\\';

	// append (const char*)
	//
	// Concatenates a path component to an existing string
	template<typename... _arguments>
	static void append(wchar_t* dest, size_t length, const char* next, const _arguments&... args)
	{
		*dest++ = delimiter;			// Append the delimiter character
		--length;						// Consumed one character

		// Convert the UTF-8 string into UTF-16 for the path
		size_t cch = MultiByteToWideChar(CP_UTF8, 0, next, -1, dest, static_cast<int>(length));
		if(cch == 0) throw Win32Exception(GetLastError());

		// MultiByteToWideChar returns the count with a null terminator when a null-
		// terminated string is provided, account for that here
		append(dest + (cch - 1), length - (cch - 1), args...);
	}

	// append (const wchar_t*)
	//
	// Concatenates a path component to an existing string
	template<typename... _arguments>
	static void append(wchar_t* dest, size_t length, const wchar_t* next, const _arguments&... args)
	{
		*dest++ = delimiter;			// Append the delimiter character
		--length;						// Consumed one character

		// Copy and increment/decrement until the null terminator is reached
		while(*next) { *dest++ = *next++; --length; }

		// Append the next path component
		append(dest, length, args...);
	}

	// append (const std::string&)
	//
	// Concatenates a path component to an existing string
	template<typename... _arguments>
	static void append(wchar_t* dest, size_t length, const std::string& next, const _arguments&... args)
	{
		*dest++ = delimiter;			// Append the delimiter character
		--length;						// Consumed one character

		// Convert the UTF-8 input string into UTF-16 for the path
		size_t cch = MultiByteToWideChar(CP_UTF8, 0, next.data(), next.length(), dest, static_cast<int>(length));
		if(cch == 0) throw Win32Exception(GetLastError());

		// MultiByteToWideChar does not include the null terminator in the count when a
		// fixed-length input string is provided
		append(dest + cch, length - cch, args...);
	}

	// append (const std::wstring&)
	//
	// Concatenates a path component to an existing string
	template<typename... _arguments>
	static void append(wchar_t* dest, size_t length, const std::wstring& next, const _arguments&... args)
	{
		*dest++ = delimiter;			// Append the delimiter character
		--length;						// Consumed one character

		// Get the length of the string data and copy it into the path
		size_t cch = next.length();
		copy(dest, length, next.data(), cch);

		// Append the next path component
		append(dest + cch, length - cch, args...);
	}

	// append (void)
	//
	// Final link in variadic template chain for append()
	static void append(wchar_t* dest, size_t length)
	{
		_ASSERTE(length == 1);		// should be exactly one character left
		*dest = 0;					// null terminate the string
	}

	// compare
	//
	// Compares two null-terminated strings
	static int compare(const wchar_t* left, const wchar_t* right)
	{
		if(left == right) return 0;
		else return wcscmp(left, right);
	}

	// copy
	//
	// Copies a source string into a destination buffer
	static void copy(wchar_t* dest, size_t length, const wchar_t* source, size_t cch)
	{
		wcsncpy_s(dest, length, source, cch);
	}

	// count (const char*)
	//
	// Counts the number of characters in an input string; does not include null terminator
	template<typename... _arguments>
	static size_t count(const char* next, const _arguments&... args)
	{
		// Determine the number of UTF-16 characters required for the string
		size_t cch = MultiByteToWideChar(CP_UTF8, 0, next, -1, nullptr, 0);
		if(cch == 0) throw Win32Exception(GetLastError());

		// MultiByteToWideChar returns the length with null termination when -1
		// has been specified for the input length; use that for the delimiter
		return cch + count(args...);
	}

	// count (const wchar_t*)
	//
	// Counts the number of characters in an input string; does not include null terminator
	template<typename... _arguments>
	static size_t count(const wchar_t* next, const _arguments&... args)
	{
		return /* delimiter */ 1 + length(next) + count(args...);
	}

	// count (const std::string&)
	//
	// Counts the number of characters in an input string; does not include null terminator
	template<typename... _arguments>
	static size_t count(const std::string& next, const _arguments&... args)
	{
		// Determine the number of UTF-16 characters required for the string
		size_t cch = MultiByteToWideChar(CP_UTF8, 0, next.data(), next.length(), nullptr, 0);
		if(cch == 0) throw Win32Exception(GetLastError());

		// MultiByteToWideChar does not include the null terminator in the count
		// when a fixed-length input string was specified, so add the delimiter
		return /* delimiter */ 1 + cch + count(args...);
	}

	// count (const std::wstring&)
	//
	// Counts the number of characters in an input string; does not include null terminator
	template<typename... _arguments>
	static size_t count(const std::wstring& next, const _arguments&... args)
	{
		return /* delimiter */ 1 + next.length() + count(args...);
	}

	// count (void)
	//
	// Final link in variadic template chain for count()
	static size_t count(void)
	{
		return /* null terminator */ 1;
	}

	// dedupe_delimiters
	//
	// Removes all duplicate delimiters in a path string
	static size_t dedupe_delimiters(wchar_t* str, size_t length)
	{
		if((*str == 0) || (length == 0)) return 0;

		// For path_format::windows the root has to be skipped to 
		// preserve constructs like \\?\ and \\SERVERNAME
		size_t offset = (skip_root(str) - str);

		size_t i = (offset) ? offset : 1;
		size_t j = i;

		while(j < length - 1)
		{
			if(str[j] == delimiter && str[i - 1] == delimiter) ++j;
			else str[i++] = str[j++];
		}

		str[i] = 0;
		return i + 1;
	}

	// find_leaf
	//
	// Locates the position of the path leaf component
	static wchar_t* find_leaf(wchar_t* str, size_t length)
	{
		if((*str == 0) || (length == 0)) return str;

		// Start at the end of the string and work backwards
		wchar_t* end = str + (length - 1);
		while((end > str) && (*end != delimiter)) --end;

		return (*end == delimiter) ? ++end : end;
	}
	
	// iterate
	//
	// Iterates a path string to the next component
	static wchar_t* iterate(wchar_t* str)
	{
		while((*str) && (*str != delimiter)) ++str;
		
		if(*str) { *str = 0; return str + 1; }
		else return str;
	}

	// length
	//
	// Determines the length of a string; does not include null terminator
	static size_t length(const wchar_t* str)
	{
		return wcslen(str);
	}

	// skip_root
	//
	// Skips over the root portion of a path
	static wchar_t* skip_root(wchar_t* str)
	{
		wchar_t* end = nullptr;			// Calculated endpoint

		if(FAILED(PathCchSkipRoot(str, &end))) return str;
		else return end;
	}

	// split
	//
	// Splits a path by inserting a null terminator in it, there must
	// be enough room to move the string right by one character
	static wchar_t* split(wchar_t* str, wchar_t* at, size_t length)
	{
		wmemmove(at + 1, at, &str[length - 1] - at);
		*at++ = str[length - 1] = 0;

		return at;
	}
};

// path_hash
//
template<typename _char_t>
struct path_hash
{
	size_t operator()(const _char_t* key) const
	{
		// http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-source

#ifndef _M_X64
		// 32-bit FNV-1a hash
		const size_t fnv_offset_basis{ 2166136261U };
		const size_t fnv_prime{ 16777619U };
#else
		// 64-bit FNV-1a hash
		const size_t fnv_offset_basis{ 14695981039346656037ULL };
		const size_t fnv_prime{ 1099511628211ULL };
#endif

		// Calcuate the FNV-1a hash for this wide character string by processing
		// each byte of the wide string individually
		size_t hash = fnv_offset_basis;

		while(*key) {

			const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(key);
			hash ^= *byteptr;
			hash *= fnv_prime;
			hash ^= *(byteptr + sizeof(uint8_t));
			hash *= fnv_prime;
			key++;
		}

		return hash;
	}
};

// path_iterator
//
template<path_format format>
class path_iterator final
{
using ops = path_operations<format>;
public:

	// Instance Constructor
	//
	path_iterator(nullptr_t) : m_current(&m_nullchar), m_remaining(&m_nullchar) 
	{
	}

	// Instance Constructor
	//
	path_iterator(const typename ops::pathchar_t* str, size_t cch) : path_iterator(nullptr)
	{
		// If the input string is null or zero length or the character count is zero, leave
		// the current and modified pointers at their defaults and do not allocate anything
		if((str != nullptr) && (*str != 0) && (cch > 0)) {

			// The string buffer allows for two additional characters, the trailing null placed 
			// on the input string and one additional slot in case of a root path split
			m_buffer = std::make_unique<typename ops::pathchar_t[]>(cch + 2);
			if(!m_buffer) throw Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

			// Load the buffer with the provided string and null out the extra characters
			ops::copy(m_buffer.get(), cch + 2, str, cch);
			m_buffer[cch] = m_buffer[cch + 1] = 0;

			// The current path component is at the start of the string
			m_current = m_buffer.get();

			// Locate the point in the path where the root ends, this may return the input
			// string (m_current) as the result if the path is not rooted
			m_remaining = ops::skip_root(m_current);

			// If a root was detected, the path must be split into two individual strings to preserve
			// that root, otherwise just iterate over the first component normally
			if(m_remaining != m_current) m_remaining = ops::split(m_current, m_remaining, cch + 2);
			else m_remaining = ops::iterate(m_current);
		}
	}

	// Copy Constructor
	//
	path_iterator(const path_iterator& rhs) : path_iterator(nullptr)
	{
		size_t cchcurrent = ops::length(rhs.m_current);
		size_t cchremaining = ops::length(rhs.m_remaining);

		// If both strings were zero-length, there is nothing to allocate or adjust
		if((cchcurrent == 0) && (cchremaining == 0)) return;

		// Allocate a new buffer that can hold both the current and remaining strings
		m_buffer = std::make_unique<typename ops::pathchar_t[]>(cchcurrent + cchremaining + 2);
		if(!m_buffer) throw Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

		if(cchcurrent) {

			// Copy the current path component into the start of the allocated buffer
			m_current = m_buffer.get();
			ops::copy(m_current, cchcurrent + 1, rhs.m_current, cchcurrent);
			m_current[cchcurrent] = 0;
		}

		if(cchremaining) {

			// Copy the remaining path string into the buffer after the current string
			m_remaining = &m_buffer[cchcurrent + 1];
			ops::copy(m_remaining, cchremaining + 1, rhs.m_remaining, cchremaining);
			m_remaining[cchremaining] = 0;
		}
	}

	// Move Constructor
	//
	path_iterator(path_iterator&& rhs) : path_iterator(nullptr)
	{
		m_buffer.swap(rhs.m_buffer);			// Swap the buffers

		// The current and remaining pointers may point to the special zero-length string
		// in the right-hand object; repoint them to this instance's member as required,
		// otherwise they will point to the same place since we took ownership of the buffer
		m_current = (rhs.m_current == &rhs.m_nullchar) ? &m_nullchar : rhs.m_current;
		m_remaining = (rhs.m_remaining == &rhs.m_nullchar) ? &m_nullchar : rhs.m_remaining;

		// The pointers in the right-hand object do not get nulled out, they get pointed
		// to that instance's special zero-length string
		rhs.m_current = &rhs.m_nullchar;
		rhs.m_remaining = &rhs.m_nullchar;
	}

	// Inequality Operator
	//
	bool operator !=(const path_iterator& rhs) const
	{
		// return true if either iterator is not at a null terminator character
		return ((m_current) && (*m_current)) || ((rhs.m_current) && (*rhs.m_current));
	}

	// Increment Operator
	//
	path_iterator& operator ++()
	{
		m_current = m_remaining;			// Move current to remaining

		// If the remaining string is not at the null terminator, iterate it
		// to point to the next path component
		if(m_remaining) m_remaining = ops::iterate(m_remaining);

		return *this;
	}

	// const pathchar_t* conversion operator
	//
	const typename ops::pathchar_t* operator*(void) const
	{
		return m_current;
	}

private:

	path_iterator& operator=(const path_iterator&)=delete;
	path_iterator& operator=(path_iterator&&)=delete;

	// m_buffer
	//
	// Path string buffer (null-terminated)
	std::unique_ptr<typename ops::pathchar_t[]>	m_buffer;

	// m_current
	//
	// Pointer to the current path component string
	typename ops::pathchar_t* m_current;

	// m_remaining
	//
	// Pointer to the remaining path string
	typename ops::pathchar_t* m_remaining;

	// m_nullchar
	//
	// Zero length string to use for null iterators
	typename ops::pathchar_t m_nullchar = 0;
};

// path
//
template<path_format format>
class path final
{
using ops = path_operations<format>;
public:

	// Instance Constructor
	//
	path(const typename ops::pathchar_t* str)
	{
		// Determine the number of characters in the input string
		size_t cch = (str) ? ops::length(str) : 0;

		// Allocate the heap buffer that will hold the input string
		m_length = cch + 1;
		m_buffer = std::make_unique<typename ops::pathchar_t[]>(m_length);
		if(!m_buffer) throw Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

		// Copy the input string into the buffer and ensure null termination
		if(str) ops::copy(m_buffer.get(), m_length, str, cch);
		m_buffer[m_length - 1] = 0;

		// Remove all duplicate delimiters from the input string
		m_length = ops::dedupe_delimiters(m_buffer.get(), m_length);
	}

	// Instance Constructor
	//
	path(const typename ops::pathchar_t* str, size_t cch)
	{
		// Allocate a heap buffer large enough for the input string and a null terminator
		m_length = (str) ? cch + 1 : 1;
		m_buffer = std::make_unique<typename ops::pathchar_t[]>(m_length);
		if(!m_buffer) throw Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

		// Copy the input string into the buffer and ensure null termination
		if(str) ops::copy(m_buffer.get(), m_length, str, cch);
		m_buffer[m_length - 1] = 0;

		// Remove all duplicate delimiters from the input string
		m_length = ops::dedupe_delimiters(m_buffer.get(), m_length);
	}

	// Instance Constructor
	//
	path(std::unique_ptr<typename ops::pathchar_t[]> buffer, size_t length) : m_buffer(std::move(buffer)), m_length(length)
	{
		// Remove all duplicate delimiters from the input string
		m_length = ops::dedupe_delimiters(m_buffer.get(), m_length);
	}

	// Copy Constructor
	//
	path(const path& rhs) : m_length(rhs.m_length)
	{
		// Allocate a new heap buffer large enough for the string and null terminator
		m_buffer = std::make_unique<typename ops::pathchar_t[]>(m_length);
		if(!m_buffer) throw Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

		// Copy the string from the right-hand object and ensure null termination
		ops::copy(m_buffer.get(), m_length, rhs.m_buffer.get(), rhs.m_length - 1);
		m_buffer[m_length - 1] = 0;
	}

	// Move Constructor
	//
	path(path&& rhs) : m_buffer(std::move(rhs.m_buffer)), m_length(rhs.m_length)
	{
		rhs.m_length = 0;
	}

	// Destructor
	//
	~path()=default;

	// Copy assignment operator
	//
	path& operator=(const path& rhs)
	{
		// Allocate a new heap buffer large enough for the string and null terminator
		m_length = rhs.m_length;
		m_buffer = std::make_unique<typename ops::pathchar_t[]>(m_length);
		if(!m_buffer) throw Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

		// Copy the string from the right-hand object and ensure null termination
		ops::copy(m_buffer.get(), m_length, rhs.m_buffer.get(), rhs.m_length - 1);
		m_buffer[m_length - 1] = 0;

		return *this;
	}

	// Move assignment operator
	//
	path& operator=(path&& rhs)
	{
		m_length = rhs.m_length;
		m_buffer.swap(rhs.m_buffer);

		rhs.m_length = 0;

		return *this;
	}

	// String assignment operator
	//
	path& operator=(const typename ops::pathchar_t* str)
	{
		// Determine the number of characters in the input string
		size_t cch = (str) ? ops::length(str) : 0;

		// Allocate the heap buffer that will hold the input string
		m_length = cch + 1;
		m_buffer = std::make_unique<typename ops::pathchar_t[]>(m_length);
		if(!m_buffer) throw Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

		// Copy the input string into the buffer and ensure null termination
		if(str) ops::copy(m_buffer.get(), m_length, str, cch);
		m_buffer[m_length - 1] = 0;

		// Remove all duplicate delimiters from the input string
		m_length = ops::dedupe_delimiters(m_buffer.get(), m_length);

		return *this;
	}

	// bool conversion operator
	//
	operator bool() const
	{
		return (m_buffer[0] != 0);
	}

	// const pathchar_t* conversion operator
	//
	operator const typename ops::pathchar_t*() const
	{
		return m_buffer.get();
	}

	// Logical not operator
	//
	bool operator!() const
	{
		return (m_buffer[0] == 0);
	}

	// Equality Operator
	//
	bool operator==(const path& rhs) const
	{
		return (ops::compare(m_buffer.get(), rhs.m_buffer.get()) == 0);
	}

	// Less-than Operator
	//
	bool operator<(const path& rhs) const
	{
		return (ops::compare(m_buffer.get(), rhs.m_buffer.get()) < 0);
	}

	// absolute
	//
	// Determines if the path is absolute or relative
	bool absolute(void) const
	{
		return (ops::skip_root(m_buffer.get()) != m_buffer.get());
	}

	// append
	//
	// Generates a new path<> instance by appending component strings
	template <typename... _arguments>
	path<format> append(const _arguments&... args) const
	{
		// Allocate a new heap buffer large enough to hold the combined strings
		size_t length = (m_length - 1) + ops::count(args...);
		auto buffer = std::make_unique<typename ops::pathchar_t[]>(length);
		if(!m_buffer) throw Win32Exception(ERROR_NOT_ENOUGH_MEMORY);

		// Start with the string represented by this path instance
		ops::copy(buffer.get(), length, m_buffer.get(), m_length - 1);

		// Append each of the provided string arguments to the buffer
		ops::append(&buffer[m_length - 1], length - (m_length - 1), args...);

		// Transfer ownership of the new buffer to a new path<> instance 
		return path<format>(std::move(buffer), length);
	}

	// begin
	//
	// Creates a path_iterator<> that can be used to iterate the path
	path_iterator<format> begin(void) const
	{
		return path_iterator<format>(m_buffer.get(), m_length);
	}

	// branch
	//
	// Generates a new path<> instance that represents the branch
	path<format> branch(void) const
	{
		// First find the position of the leaf within the path string
		typename ops::pathchar_t* leaf = ops::find_leaf(m_buffer.get(), m_length);

		// Determine where to end the branch string; if the branch is not the root
		// any trailing delimiter should be removed from the resultant path<>
		size_t pos = leaf - m_buffer.get();
		if((pos) && (leaf > ops::skip_root(m_buffer.get())) && (m_buffer[pos - 1] == ops::delimiter)) --pos;

		return path<format>(m_buffer.get(), pos);
	}

	// end
	//
	// Creates a path_iterator<> that can be used to stop an iteration
	path_iterator<format> end(void) const
	{
		return path_iterator<format>(nullptr);
	}

	// leaf
	//
	// Generates a new path<> instance that represents the leaf
	path<format> leaf(void) const
	{
		typename ops::pathchar_t* leaf = ops::find_leaf(m_buffer.get(), m_length);
		return path<format>(leaf, m_length - (leaf - m_buffer.get()));
	}

private:

	// m_buffer
	//
	// Path string buffer (null-terminated)
	std::unique_ptr<typename ops::pathchar_t[]> m_buffer;

	// m_length
	//
	// Length of the buffer in characters; includes null terminator
	size_t m_length;
};

// posix_path alias
//
using posix_path = path<path_format::posix>;

// windows_path alias
//
using windows_path = path<path_format::windows>;

// std namespace specializations
//
namespace std {

	// std::equal_to<posix_path>
	//
	template<> struct equal_to<posix_path>
	{
		bool operator()(const posix_path& lhs, const posix_path& rhs) const { return lhs == rhs; }
	};

	// std::equal_to<windows_path>
	//
	template<> struct equal_to<windows_path>
	{
		bool operator()(const windows_path& lhs, const windows_path& rhs) const { return lhs == rhs; }
	};

	// std::hash<posix_path>
	//
	template<> struct hash<posix_path> : public path_hash<path_operations<path_format::posix>::pathchar_t>
	{
	};

	// std::hash<windows_path>
	//
	template<> struct hash<windows_path> : public path_hash<path_operations<path_format::windows>::pathchar_t>
	{
	};

	// std::less<path_format::posix>>
	//
	template<> struct less<posix_path>
	{
		bool operator()(const posix_path& lhs, const posix_path& rhs) const { return lhs < rhs; }
	};

	// std::less<path_format::windows>>
	//
	template<> struct less<windows_path>
	{
		bool operator()(const windows_path& lhs, const windows_path& rhs) const { return lhs < rhs; }
	};

}	// namespace std

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PATH_H_