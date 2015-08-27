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
#include "Exception.h"

#pragma warning(push, 4)

// Exception::s_module
//
// Defaults to the process module handle if there are message resources present
HMODULE Exception::s_module = Exception::ModuleHasMessageTable(GetModuleHandle(nullptr)) ? 
	GetModuleHandle(nullptr) : nullptr;

//-----------------------------------------------------------------------------
// Exception Copy Constructor

Exception::Exception(const Exception& rhs) 
{
	m_hresult = rhs.m_hresult;
	m_message = rhs.m_message;
	m_what = rhs.m_what;
	m_inner = (rhs.m_inner) ? new Exception(*rhs.m_inner) : nullptr;
}

//-----------------------------------------------------------------------------
// Exception Destructor

Exception::~Exception()
{
	if(m_inner) delete m_inner;
}

//-----------------------------------------------------------------------------
// Exception::operator=

Exception& Exception::operator=(const Exception& rhs)
{
	m_hresult = rhs.m_hresult;
	m_message = rhs.m_message;
	m_what = rhs.m_what;
	
	// Inner exceptions are pointer-based; delete before changing it
	if(m_inner) delete m_inner; 
	m_inner = (rhs.m_inner) ? new Exception(*rhs.m_inner) : nullptr;

	return *this;
}

//-----------------------------------------------------------------------------
// Exception::FormatMessage (private)
//
// Specialized version of ::FormatMessage that formats the string
//
// Arguments:
//
//	hresult			- Exception HRESULT code
//	langid			- Exception message table language id
//	module			- Module containing the message table resources
//	args			- Array of message insertion arguments

tchar_t* Exception::FormatMessage(HRESULT hresult, DWORD langid, HMODULE module, const DWORD_PTR* args)
{
	tchar_t* out = nullptr;					// Allocated buffer from ::FormatMessage
	DWORD result;							// Result from ::FormatMessage

	// Generate the flags for ::FormatMessage based on presence of the HMODULE and DWORD_PTR array
	DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | ((module) ? FORMAT_MESSAGE_FROM_HMODULE : 0) |
		((args) ? FORMAT_MESSAGE_ARGUMENT_ARRAY : FORMAT_MESSAGE_IGNORE_INSERTS);

	__try { 
		
		// Attempt to format the message using the flags determined above
		result = ::FormatMessage(flags, module, static_cast<DWORD>(hresult), langid, reinterpret_cast<tchar_t*>(&out), 0, 
			reinterpret_cast<va_list*>(const_cast<DWORD_PTR*>(args))); 
	}

	// If arguments were used improperly, an access violation is a common reason for an exception
	__except((args != nullptr) && (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)) {

		// Release the buffer if it was successfully allocated
		if(out) { LocalFree(out); out = nullptr; }

		// Try the operation again, ignoring the arguments/insert strings
		result = ::FormatMessage(flags | FORMAT_MESSAGE_IGNORE_INSERTS, module, static_cast<DWORD>(hresult), langid,
			reinterpret_cast<tchar_t*>(&out), 0, reinterpret_cast<va_list*>(const_cast<DWORD_PTR*>(args)));
	}

	// Release the buffer if it was allocated and FormatMessage() didn't succeed
	if((result == 0) && (out)) { LocalFree(out); return nullptr; }

	return out;
}

//-----------------------------------------------------------------------------
// Exception::GetDefaultMessage (protected)
//
// Invoked when an HRESULT cannot be mapped to a message table string
//
// Arguments:
//
//	hresult		- Thrown HRESULT code

std::tstring Exception::GetDefaultMessage(const HRESULT& hresult)
{
	tchar_t buffer[256];			// Stack buffer to hold formatted string

	// Format a default message for the HRESULT that just breaks out the indiviual values
	_sntprintf_s(buffer, 256, _TRUNCATE, _T("HRESULT 0x%08X [Severity: 0x%02X (%d), Facility: 0x%04X (%d), Code: 0x%04X (%d)]"), 
		hresult, HRESULT_SEVERITY(hresult), HRESULT_SEVERITY(hresult), HRESULT_FACILITY(hresult), HRESULT_FACILITY(hresult), 
		HRESULT_CODE(hresult), HRESULT_CODE(hresult));

	return std::tstring(buffer);
}

//-----------------------------------------------------------------------------
// Exception::ModuleHasMessageTable (static)
//
// Determines if the specified module handle contains message resource(s)
//
// Arguments:
//
//	module		- Module handle to test for message table resources

bool Exception::ModuleHasMessageTable(HMODULE module)
{
	HMODULE	result = nullptr;				// Result from enumeration

	// Use EnumResourceTypes to determine if there are messages in the module
	EnumResourceTypes(module, [](HMODULE module, LPTSTR type, LONG_PTR param) -> BOOL {

		// If an RT_MESSAGETABLE resource is found, return the module handle back
		// to the caller through param and stop the enumeration by returning FALSE
		*reinterpret_cast<HMODULE*>(param) = nullptr;
		if(type == RT_MESSAGETABLE) { *reinterpret_cast<HMODULE*>(param) = module; return FALSE; }
		else return TRUE;

	}, reinterpret_cast<LONG_PTR>(&result));

	return (result != nullptr);
}

//-----------------------------------------------------------------------------
// Exception::SetExceptionMessage
//
// Final variadic function in the chain; sets the contained message strings
//
// Arguments:
//
//	hresult		- HRESULT propagated from the constructor
//	module		- Module handle to use for message resources, or nullptr
//	args		- Collection of insert argument pointers

void Exception::SetExceptionMessage(const HRESULT& hresult, HMODULE module, std::vector<DWORD_PTR>& args)
{
	// Invoke the specialized FormatMessage() function to format the exception message
	tchar_t* message = FormatMessage(hresult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), module, args.data());

	// If a message string was returned, use that, otherwise invoke GetDefaultMessage()
	if(message) { m_message = message; LocalFree(message); }
	else m_message = GetDefaultMessage(hresult);

	// std::exception::what() is stored separately since it must always be ANSI, regardless of tchar_t type
	m_what = std::to_string(m_message);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
