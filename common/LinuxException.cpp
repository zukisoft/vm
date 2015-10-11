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

#include "stdafx.h"
#include "LinuxException.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// LinuxException Constructor
//
// Arguments:
//
//	result		- Linux/POSIX result code

LinuxException::LinuxException(int result) : m_code(result), m_what(AllocateMessage(result)), m_owned((m_what != nullptr)) 
{
}

//-----------------------------------------------------------------------------
// LinuxException Constructor
//
// Arguments:
//
//	result		- Linux/POSIX result code
//	inner		- Inner std::exception object reference

LinuxException::LinuxException(int result, std::exception const& inner) : m_code(result), m_what(AllocateMessage(result)), m_owned((m_what != nullptr)), m_inner(inner)
{
}

//-----------------------------------------------------------------------------
// LinuxException Copy Constructor

LinuxException::LinuxException(LinuxException const& rhs) : m_code(rhs.m_code), m_what(rhs.m_what), m_owned(false), m_inner(rhs.m_inner)
{
}

//-----------------------------------------------------------------------------
// LinuxException Destructor

LinuxException::~LinuxException()
{
	if(m_owned && (m_what != nullptr)) LocalFree(m_what);
}

//-----------------------------------------------------------------------------
// LinuxException::AllocateMessage (private, static)
//
// Generates the formatted message string from the project resources
//
// Arguments:
//
//	result		- Linux/POSIX result code for which to generate the message

char* LinuxException::AllocateMessage(int result)
{
	LPTSTR message = nullptr;					// Allocated string from ::FormatMessage

	// Attempt to format the message from the current module resources
	DWORD cchmessage = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, nullptr,
		static_cast<DWORD>(HRESULT_FROM_LINUX(result)), GetThreadUILanguage(), reinterpret_cast<LPTSTR>(&message), 0, nullptr); 
	if(cchmessage == 0) {

		// The message could not be looked up in the specified module; generate the default message instead
		if(message) { LocalFree(message); message = nullptr; }
		cchmessage = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, 
			s_defaultformat, 0, 0, reinterpret_cast<LPTSTR>(&message), 0, reinterpret_cast<va_list*>(&result));
		if(cchmessage == 0) {

			// The default message could not be generated; give up
			if(message) ::LocalFree(message);
			return nullptr;
		}
	}

#ifdef _UNICODE
	// UNICODE projects need to convert the message string into CP_UTF8 or CP_ACP
	int convertcch = ::WideCharToMultiByte(CP_UTF8, 0, message, cchmessage, nullptr, 0, nullptr, nullptr);
	char* converted = reinterpret_cast<char*>(::LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, (convertcch + 1) * sizeof(char)));
	if(converted) ::WideCharToMultiByte(CP_UTF8, 0, message, cchmessage, converted, convertcch, nullptr, nullptr);

	LocalFree(message);
	return converted;
#else
	return message;
#endif
}

//-----------------------------------------------------------------------------
// LinuxException::getCode
//
// Gets the Linux/POSIX result code

int LinuxException::getCode(void) const
{
	return m_code;
}

//-----------------------------------------------------------------------------
// LinuxException::getInnerException
//
// Exposes a reference to the inner exception

std::exception const& LinuxException::getInnerException(void) const
{
	return m_inner;
}

//-----------------------------------------------------------------------------
// LinuxException::what
//
// Gets a pointer to the exception message text
//
// Arguments:
//
//	NONE

char const* LinuxException::what(void) const
{
	return m_what;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
