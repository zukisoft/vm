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

#include "stdafx.h"						// Include project pre-compiled headers
#include "ElfArguments.h"				// Include ELFArguments declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Explicit Instantiations

#ifdef _M_X64
template ElfArgumentsT<Elf64_Addr, Elf64_auxv_t>;
#else
template ElfArgumentsT<Elf32_Addr, Elf32_auxv_t>;
#endif

//---------------------------------------------------------------------------
// ElfArgumentsT Constructor
//
// Arguments:
//
//	NONE

template <class addr_t, class auxv_t>
ElfArgumentsT<addr_t, auxv_t>::ElfArgumentsT() :
	m_info(MemoryRegion::Reserve(MemoryRegion::AllocationGranularity, MEM_COMMIT | MEM_TOP_DOWN))
{
	m_offset = 0;
}

//---------------------------------------------------------------------------
// ElfArgumentsT::AlignUp (private, static)
//
// Aligns an offset up to the specified alignment
//
// Arguments:
//
//	offset		- Offset to be aligned
//	alignment	- Alignment

template <class addr_t, class auxv_t>
size_t ElfArgumentsT<addr_t, auxv_t>::AlignUp(size_t offset, size_t alignment)
{
	if(alignment < 1) throw Exception(E_ARGUMENTOUTOFRANGE, _T("alignment"));

	if(offset == 0) return 0;
	else return offset + ((alignment - (offset % alignment)) % alignment);
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendArgument
//
// Appends an argument to the contained argument list
//
// Arguments:
//
//	value		- Command-line argument to be added

template <class addr_t, class auxv_t>
void ElfArgumentsT<addr_t, auxv_t>::AppendArgument(const char* value)
{
	if(!value) throw Exception(E_ARGUMENTNULL, _T("value"));
	
	m_argv.push_back(AppendInfo(value, strlen(value) + 1));
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendArgument
//
// Appends an argument to the contained argument list
//
// Arguments:
//
//	value		- Command-line argument to be added

template <class addr_t, class auxv_t>
void ElfArgumentsT<addr_t, auxv_t>::AppendArgument(const wchar_t* value)
{
	if(!value) throw Exception(E_ARGUMENTNULL, _T("value"));

	// Get the number of bytes required to hold the converted string
	int required = WideCharToMultiByte(CP_UTF8, 0, value, -1, NULL, 0, NULL, NULL);
	if(required == 0) throw Win32Exception();
	
	// Reserve space in the information block for the UTF8 converted string and convert it
	addr_t arg = AppendInfo(NULL, required);
	WideCharToMultiByte(CP_UTF8, 0, value, -1, reinterpret_cast<char*>(arg), required, NULL, NULL);

	m_argv.push_back(arg);
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendAuxiliaryVector
//
// Appends an auxiliary vector to the contained list
//
// Arguments:
//
//	type		- Auxiliary vector type code
//	value		- Auxiliary vector value

template <class addr_t, class auxv_t>
void ElfArgumentsT<addr_t, auxv_t>::AppendAuxiliaryVector(addr_t type, addr_t value)
{
	m_auxv.push_back(auxv_t(type, value));
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendAuxiliaryVector
//
// Appends an auxiliary vector to the contained list
//
// Arguments:
//
//	type		- Auxiliary vector type code
//	value		- Auxiliary vector value

template <class addr_t, class auxv_t>
void ElfArgumentsT<addr_t, auxv_t>::AppendAuxiliaryVector(addr_t type, const char* value)
{
	auxv_t vector(type);				// Initialize with a NULL/zero value

	if(value) vector.a_val = AppendInfo(value, strlen(value) + 1); 
	m_auxv.push_back(vector);
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendAuxiliaryVector
//
// Appends an auxiliary vector to the contained list
//
// Arguments:
//
//	type		- Auxiliary vector type code
//	value		- Auxiliary vector value

template <class addr_t, class auxv_t>
void ElfArgumentsT<addr_t, auxv_t>::AppendAuxiliaryVector(addr_t type, const wchar_t* value)
{
	auxv_t vector(type);				// Initialize with a NULL/zero value

	if(value) {
	
		// Get the number of bytes required to hold the converted string
		int required = WideCharToMultiByte(CP_UTF8, 0, value, -1, NULL, 0, NULL, NULL);
		if(required == 0) throw Win32Exception();

		// Reserve space in the information block for the UTF8 converted string and convert it
		vector.a_val = AppendInfo(NULL, required);
		WideCharToMultiByte(CP_UTF8, 0, value, -1, reinterpret_cast<char*>(vector.a_val), required, NULL, NULL);
	}

	m_auxv.push_back(vector);
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendAuxiliaryVector
//
// Appends an auxiliary vector to the contained list
//
// Arguments:
//
//	type		- Auxiliary vector type code
//	buffer		- Buffer holding binary auxiliary vector data
//	length		- Length of the input buffer

template <class addr_t, class auxv_t>
void ElfArgumentsT<addr_t, auxv_t>::AppendAuxiliaryVector(addr_t type, const void* buffer, size_t length)
{
	auxv_t vector(type);				// Initialize with a NULL/zero value

	if(buffer) vector.a_val = AppendInfo(buffer, length);
	m_auxv.push_back(vector);
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendEnvironmentVariable
//
// Appends a preformatted environment variable to the contained list
//
// Arguments:
//
//	variable	- Preformatted environment variable in KEY=VALUE format

template <class addr_t, class auxv_t>
void ElfArgumentsT<addr_t, auxv_t>::AppendEnvironmentVariable(const char* keyandvalue)
{
	if(!keyandvalue) throw Exception(E_ARGUMENTNULL, _T("keyandvalue"));

	// Since the variable is preformatted, it can be put right into place
	m_env.push_back(AppendInfo(keyandvalue, strlen(keyandvalue) + 1));
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendEnvironmentVariable
//
// Appends a preformatted environment variable to the contained list
//
// Arguments:
//
//	variable	- Preformatted environment variable in KEY=VALUE format

template <class addr_t, class auxv_t>
void ElfArgumentsT<addr_t, auxv_t>::AppendEnvironmentVariable(const wchar_t* keyandvalue)
{
	if(!keyandvalue) throw Exception(E_ARGUMENTNULL, _T("keyandvalue"));

	// Get the number of bytes required to hold the converted string and the NULL terminator
	int required = WideCharToMultiByte(CP_UTF8, 0, keyandvalue, -1, NULL, 0, NULL, NULL);
	if(required == 0) throw Win32Exception();

	// Append the converted string to the information block
	addr_t variable = AppendInfo(NULL, required);
	WideCharToMultiByte(CP_UTF8, 0, keyandvalue, -1, reinterpret_cast<char*>(variable), required, NULL, NULL);

	m_env.push_back(variable);				// Keep track of the pointer
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendEnvironmentVariable
//
// Appends an environment variable to the contained list
//
// Arguments:
//
//	key			- Environment variable key
//	value		- Environment variable value

template <class addr_t, class auxv_t>
void ElfArgumentsT<addr_t, auxv_t>::AppendEnvironmentVariable(const char* key, const char* value)
{
	if(!key) throw Exception(E_ARGUMENTNULL, _T("key"));

	// Append the key and an equal sign to the information block
	addr_t variable = AppendInfo(key, strlen(key));
	AppendInfo("=", 1);

	// If a value was specifed, append that and a NULL terminator, otherwise
	// just append a NULL terminator after the equal sign
	if(value) AppendInfo(value, strlen(value) + 1);
	else AppendInfo(NULL, 1);
	
	m_env.push_back(variable);
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendEnvironmentVariable
//
// Appends an environment variable to the contained list
//
// Arguments:
//
//	key			- Environment variable key
//	value		- Environment variable value

template <class addr_t, class auxv_t>
void ElfArgumentsT<addr_t, auxv_t>::AppendEnvironmentVariable(const wchar_t* key, const wchar_t* value)
{
	if(!key) throw Exception(E_ARGUMENTNULL, _T("key"));

	// Get the number of bytes required to hold the converted key string
	int required = WideCharToMultiByte(CP_UTF8, 0, key, wcslen(key), NULL, 0, NULL, NULL);
	if(required == 0) throw Win32Exception();

	// Append the converted key and an equal sign to the information block
	addr_t variable = AppendInfo(NULL, required);
	WideCharToMultiByte(CP_UTF8, 0, key, wcslen(key), reinterpret_cast<char*>(variable), required, NULL, NULL);
	AppendInfo("=", 1);

	// If a value was specified, convert and append that with it's NULL terminator
	if(value) {

		required = WideCharToMultiByte(CP_UTF8, 0, value, -1, NULL, 0, NULL, NULL);
		if(required == 0) throw Win32Exception();

		addr_t valueptr = AppendInfo(NULL, required);
		WideCharToMultiByte(CP_UTF8, 0, value, -1, reinterpret_cast<char*>(valueptr), required, NULL, NULL);
	}

	else AppendInfo(NULL, 1);				// No value --> just a NULL terminator

	m_env.push_back(variable);
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendInfo (private)
//
// Appends data to the information block
//
// Arguments:
//
//	value		- addr_t value to be appended

template <class addr_t, class auxv_t>
addr_t ElfArgumentsT<addr_t, auxv_t>::AppendInfo(addr_t value)
{
	if((m_offset + sizeof(addr_t)) > m_info->Length) throw Exception(E_OUTOFMEMORY);

	addr_t destination = static_cast<addr_t>(uintptr_t(m_info->Pointer) + m_offset);
	*reinterpret_cast<addr_t*>(destination) = value;

	m_offset += sizeof(addr_t);
	return destination;
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendInfo (private)
//
// Appends data to the information block
//
// Arguments:
//
//	value		- auxv_t value to be appended

template <class addr_t, class auxv_t>
addr_t ElfArgumentsT<addr_t, auxv_t>::AppendInfo(auxv_t value)
{
	if((m_offset + sizeof(auxv_t)) > m_info->Length) throw Exception(E_OUTOFMEMORY);

	addr_t destination = static_cast<addr_t>(uintptr_t(m_info->Pointer) + m_offset);
	*reinterpret_cast<auxv_t*>(destination) = value;

	m_offset += sizeof(auxv_t);
	return destination;
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::AppendInfo (private)
//
// Appends data to the information block
//
// Arguments:
//
//	buffer		- Buffer with data to be appended or NULL to reserve
//	length		- Length of the data to be appended

template <class addr_t, class auxv_t>
addr_t ElfArgumentsT<addr_t, auxv_t>::AppendInfo(const void* buffer, size_t length)
{
	if((m_offset + length) > m_info->Length) throw Exception(E_OUTOFMEMORY);

	// Use the current offset into the block as the destination and copy the data
	addr_t destination = static_cast<addr_t>(uintptr_t(m_info->Pointer) + m_offset);
	if(buffer) memcpy(reinterpret_cast<void*>(destination), buffer, length);
	
	m_offset += length;
	return destination;
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::CreateArgumentVector
//
// Creates the vector of values that can be copied onto the stack prior to
// jumping into an ELF entry point
//
// Arguments:
//
//	length		- Receives the length of the vector, in bytes

template <class addr_t, class auxv_t>
const void* ElfArgumentsT<addr_t, auxv_t>::CreateArgumentVector(size_t* length)
{
	if(!length) throw Exception(E_ARGUMENTNULL, _T("length"));

	// ALIGNMENT
	m_offset = AlignUp(m_offset, 16);
	if(m_offset > m_info->Length) throw Exception(E_OUTOFMEMORY);

	// ARGC / ARGV
	uintptr_t begin = uintptr_t(AppendInfo(static_cast<addr_t>(m_argv.size())));
	for_each(m_argv.begin(), m_argv.end(), [&](addr_t& addr) { AppendInfo(addr); });
	AppendInfo(static_cast<addr_t>(NULL));

	// ENVIRONMENT VARIABLES
	for_each(m_env.begin(), m_env.end(), [&](addr_t addr) { AppendInfo(addr); });
	AppendInfo(static_cast<addr_t>(NULL));

	// AUXILIARY VECTORS
	for_each(m_auxv.begin(), m_auxv.end(), [&](auxv_t auxv) { AppendInfo(auxv); });
	AppendInfo(auxv_t(AT_NULL, 0));

	// TERMINATOR
	AppendInfo(static_cast<addr_t>(NULL));

	// ALIGNMENT
	m_offset = AlignUp(m_offset, 16);
	if(m_offset > m_info->Length) throw Exception(E_OUTOFMEMORY);

	// Calculate the end address and the overall length
	uintptr_t end = uintptr_t(m_info->Pointer) + m_offset;
	*length = (end - begin);

	return reinterpret_cast<const void*>(begin);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
