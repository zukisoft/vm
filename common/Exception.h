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

#ifndef __EXCEPTION_H_
#define __EXCEPTION_H_
#pragma once

#include <exception>
#include <functional>
#include <string>
#include <vector>
#include <stdint.h>
#include <Windows.h>
#include "generic_text.h"

#pragma warning(push, 4)	

//-----------------------------------------------------------------------------
// Class Exception
//
// Custom HRESULT-based exception class that reads the message associated with
// the HRESULT from a resource message table. When specifying a module handle,
// that module must contain a message table to prevent a NULL message from being
// set.  Callers can verify if a module has the necessary resources by passing the
// handle to the static ModuleHasMessages() function.
//
// By default, Exception will try to use the handle returned by GetModuleHandle(NULL)
// to access message table resources.  This default can be overridden at any time
// by calling the static SetDefaultMessagesModule() function.
//
// Constructors:
//
//	Exception(HRESULT, ...)
//		- Attempts to load the message for HRESULT with optional insert arguments
//
//	Exception(HRESULT, Exception const&, ...)
//		- Attempts to load the message for HRESULT with optional insert arguments
//		- Copies the inner exception; accessible via .InnerException
//
//	Exception(HRESULT, HMODULE, ...)
//		- Attempts to load the message for HRESULT with optional insert arguments
//		- Uses a specific module handle rather than the default
//
//	Exception(HRESULT, HMODULE, Exception const&, ...)
//		- Attempts to load the message for HRESULT with optional insert arguments
//		- Uses a specific module handle rather than the default
//		- Copies the inner exception; accessible via .InnerException

class Exception : public std::exception
{
public:

	// Instance Constructor (HRESULT)
	//
	template <typename... _insertions>
	Exception(HRESULT const& hresult, _insertions const&... insertions) : Exception{ hresult, s_module, insertions... } {}

	// Instance Constructor (HRESULT + Inner Exception)
	//
	template <typename... _insertions>
	Exception(HRESULT const& hresult, Exception const& inner, _insertions const&... insertions) : Exception{ hresult, s_module, inner, insertions... } {}

	// Instance Constructor (HRESULT + HMODULE)
	//
	template <typename... _insertions>
	Exception(HRESULT const& hresult, HMODULE const& module, _insertions const&... insertions)
	{
		m_hresult = hresult;				// Store the thrown error code

		// Process the HRESULT and the insertions into a single message string
		std::vector<DWORD_PTR> args;
		SetExceptionMessage(hresult, module, args, insertions...);
	}

	// Instance Constructor (HRESULT + HMODULE + Inner Exception)
	//
	template <typename... _insertions>
	Exception(HRESULT const& hresult, HMODULE const& module, Exception const& inner, _insertions const&... insertions)
	{
		m_hresult = hresult;				// Store the thrown error code
		m_inner = new Exception(inner);		// Create a copy of the inner exception

		// Process the HRESULT and the insertions into a single message string
		std::vector<DWORD_PTR> args;
		SetExceptionMessage(hresult, module, args, insertions...);
	}

	// Destructor
	//
	virtual ~Exception();

	// Copy Constructor
	//
	Exception(Exception const& rhs);

	// Assignment operator
	//
	Exception& operator=(Exception const& rhs);
	
	// SetDefaultMessagesModule (static)
	//
	// Alters the module handle used to look up message table strings
	static void SetDefaultMessagesModule(HMODULE module) { s_module = module; }

	// ModuleHasMessageResources (static)
	//
	// Determines if the specified module handle can access message resources
	static bool ModuleHasMessageTable(HMODULE hmodule);

	// what (std::exception)
	//
	// Gets the exception message text
	virtual char_t const* what(void) const { return m_what.c_str(); }

	// Code
	//
	// Gets the error code associated with the HRESULT
	__declspec(property(get=getCode)) uint16_t Code;
	uint16_t getCode(void) const { return HRESULT_CODE(m_hresult); }

	// Facility
	//
	// Gets the facility code associated with the HRESULT
	__declspec(property(get=getFacility)) uint16_t Facility;
	uint16_t getFacility(void) const { return HRESULT_FACILITY(m_hresult); }

	// HResult
	//
	// Gets the complete HRESULT code for this exception object
	__declspec(property(get=getHResult)) HRESULT HResult;
	HRESULT getHResult(void) const { return m_hresult; }

	// InnerException
	//
	// Accesses the inner exception; may be NULL
	__declspec(property(get=getInnerException)) Exception const* InnerException;
	Exception const* getInnerException(void) const { return m_inner; }

	// Message
	//
	// Gets the exception message text
	__declspec(property(get=getMessage)) tchar_t const* Message;
	tchar_t const* getMessage(void) const { return m_message.c_str(); }

	// Severity
	//
	// Gets the severity code associated with the HRESULT
	__declspec(property(get=getSeverity)) uint8_t Severity;
	uint8_t getSeverity(void) const { return HRESULT_SEVERITY(m_hresult); }

protected:

	// GetDefaultMessage
	//
	// Invoked when an HRESULT code cannot be mapped to a message table string
	virtual std::tstring GetDefaultMessage(HRESULT const& hresult);

private:

	Exception()=delete;

	// FormatMessage (static)
	//
	// Specialization of ::FormatMessage specifically for Exception
	static tchar_t* FormatMessage(HRESULT hresult, DWORD langid, HMODULE module, DWORD_PTR const* args);

	// SetExceptionMessage
	//
	// Variadic function used to process each message insert parameter
	template<typename _first, typename... _insertions>
	void SetExceptionMessage(HRESULT const& hresult, HMODULE module, std::vector<DWORD_PTR>& args, _first const& first, _insertions const&... remaining)
	{
		// FormatMessage() does not support floating-point values as insertion arguments
		static_assert(!std::is_floating_point<_first>::value, "Floating point values cannot be specified as Exception message insert arguments");

		// FormatMessage() does not support insertion arguments larger than DWORD_PTR in size when using an argument array
		static_assert(!(std::is_integral<_first>::value && sizeof(_first) > sizeof(DWORD_PTR)), 
			"Integral values larger than DWORD_PTR in size cannot be specified as Exception message insert arguments");

		// Insert the argument into the vector<> and recursively invoke ourselves for remaining arguments
		args.push_back((DWORD_PTR)first);
		SetExceptionMessage(hresult, module, args, remaining...);
	}

	// SetExceptionMessage
	//
	// Final variadic function in the chain; sets the contained message strings
	void SetExceptionMessage(HRESULT const& hresult, HMODULE module, std::vector<DWORD_PTR>& args);

	// m_hresult
	//
	// Exception HRESULT code
	HRESULT m_hresult = S_OK;

	// m_inner
	//
	// Pointer to an inner exception instance
	Exception* m_inner = nullptr;

	// m_message
	//
	// Generic text exception message string
	std::tstring m_message;

	// m_what
	//
	// ANSI text exception message string
	std::string m_what;

	// s_module
	//
	// Module handle used to load message resources
	static HMODULE s_module;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EXCEPTION_H_
