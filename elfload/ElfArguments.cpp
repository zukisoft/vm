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
	auxv_t vector = { type, value };
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
void ElfArgumentsT<addr_t, auxv_t>::AppendAuxiliaryVector(addr_t type, const char* value)
{
	auxv_t vector = { type, 0 };				// Initialize with a NULL/zero value

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
	auxv_t vector = { type, 0 };				// Initialize with a NULL/zero value

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
	auxv_t vector = { type, 0 };				// Initialize with a NULL/zero value

	if(buffer) vector.a_val = AppendInfo(buffer, length);
	m_auxv.push_back(vector);
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
// ElfArgumentsT::CreateArgumentStack
//
// Creates the vector of values that can be pushed onto the stack prior to
// jumping into an ELF entry point.  Values will be returned in the order
// they should be pushed onto the stack.
//
// Arguments:
//
//	stack		- Pointer to receive the vector

template <class addr_t, class auxv_t>
size_t ElfArgumentsT<addr_t, auxv_t>::CreateArgumentStack(addr_t** stack)
{
	if(!stack) throw Exception(E_ARGUMENTNULL, _T("stack"));

	// TODO: verify 16 bytes is a multiple of addr_t
	///if(sizeof(

	// Calculate size of pointer data to be pushed on the stack
	size_t alloc = 
		sizeof(addr_t) +						// argc
		(sizeof(addr_t) * m_argv.size()) + 		// argv pointers
		sizeof(addr_t) +						// NULL
		(sizeof(addr_t) * m_env.size()) +		// env pointers
		sizeof(addr_t) +						// NULL
		(sizeof(auxv_t) * m_auxv.size()) +		// auxiliary vectors
		sizeof(auxv_t) +						// AT_NULL
		sizeof(addr_t);							// NULL

	// The stack must be aligned on 16-byte boundaries
	size_t alignedalloc = AlignUp(alloc, 16);

	// Allocate the memory for the entire block off the process heap
	uint8_t* block = new uint8_t[alignedalloc];
	if(!block) throw Exception(E_OUTOFMEMORY);

	memset(block, 0, alignedalloc);				// Initialize to all NULLs

	// Work backwards to load all the data into the buffer so that when
	// it's pushed onto the stack it's in the correct order

	// PADDING
	size_t offset = alignedalloc - alloc;

	// NULL
	offset += sizeof(addr_t);

	// NULL AUXILIARY VECTOR
	auxv_t auxvterm = { AT_NULL, 0 };
	*reinterpret_cast<auxv_t*>(block + offset) = auxvterm;
	offset += sizeof(auxv_t);

	// AUXILIARY VECTORS
	for_each(m_auxv.rbegin(), m_auxv.rend(), [&](auxv_t& auxv) {

		*reinterpret_cast<auxv_t*>(block + offset) = auxv;
		offset += sizeof(auxv_t);
	});

	// NULL
	offset += sizeof(addr_t);

	// ENVIRONMENT VARIABLES
	for_each(m_env.rbegin(), m_env.rend(), [&](addr_t& env) {

		*reinterpret_cast<addr_t*>(block + offset) = env;
		offset += sizeof(addr_t);
	});

	// NULL
	offset += sizeof(addr_t);

	// COMMAND LINE ARGUMENTS
	for_each(m_argv.rbegin(), m_argv.rend(), [&](addr_t& arg) {

		*reinterpret_cast<addr_t*>(block + offset) = arg;
		offset += sizeof(addr_t);
	});
	

	// NUMBER OF COMMAND LINE ARGUMENTS
	*reinterpret_cast<addr_t*>(block + offset) = m_argv.size();
	offset += sizeof(addr_t);

	// Verify that the exact end of the allocated memory has been reached
	if(offset != alignedalloc) throw Exception(E_UNEXPECTED);	// <--- TODO

	// The first values to pe pushed onto the stack are at the base pointer
	*stack = reinterpret_cast<addr_t*>(block);

	// Return the number of values that need to be pushed onto the stack
	return (alignedalloc / sizeof(addr_t));
}

//-----------------------------------------------------------------------------
// ElfArgumentsT::ReleaseArgumentStack (static)
//
// Releases the memory allocated from CreateArgumentStack
//
// Arguments:
//
//	stack		- Pointer allocated by CreateVector()

template <class addr_t, class auxv_t>
addr_t* ElfArgumentsT<addr_t, auxv_t>::ReleaseArgumentStack(addr_t* stack)
{
	if(stack) delete[] reinterpret_cast<uint8_t*>(stack);
	return nullptr;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
