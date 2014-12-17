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

#include "stdafx.h"
#include "ElfArguments.h"

#pragma warning(push, 4)

// Explicit Instantiations
//
template void* ElfArguments::GenerateProcessStack<ElfClass::x86>(HANDLE, void*, size_t);
#ifdef _M_X64
template void* ElfArguments::GenerateProcessStack<ElfClass::x86_64>(HANDLE, void*, size_t);
#endif

//-----------------------------------------------------------------------------
// ::BufferWrite
//
// Helper function for ElfArguments::GenerateStackImage, writes a value of
// a specific data type into a buffer and returns an updated pointer
//
// Arguments:
//
//	dest		- Destination buffer pointer
//	source		- Source value

template <typename _type>
inline uint8_t* BufferWrite(uint8_t* dest, const _type& source)
{
	*reinterpret_cast<_type*>(dest) = source;
	return dest + sizeof(_type);
}

//---------------------------------------------------------------------------
// ElfArguments Constructor
//
// Arguments:
//
//	argc		- Pointer to the initial set of command line arguments
//	envp		- Pointer to the initial set of environment variables

ElfArguments::ElfArguments(const uapi::char_t** argv, const uapi::char_t** envp)
{
	// Iterate over any provided arguments/variables and append them
	while(argv && *argv) { AppendArgument(*argv); ++argv; }
	while(envp && *envp) { AppendEnvironmentVariable(*envp); ++envp; }
}

//-----------------------------------------------------------------------------
// ElfArguments::AppendArgument
//
// Appends an argument to the contained argument list
//
// Arguments:
//
//	value		- Command-line argument to be added

void ElfArguments::AppendArgument(const uapi::char_t* value)
{
	if(!value) throw Exception(E_ARGUMENTNULL, "value");
	m_argv.push_back(AppendInfo(value, strlen(value) + 1));
}

//-----------------------------------------------------------------------------
// ElfArguments::AppendAuxiliaryVector
//
// Appends an auxiliary vector to the contained list
//
// Arguments:
//
//	type		- Auxiliary vector type code
//	value		- Auxiliary vector value

void ElfArguments::AppendAuxiliaryVector(int type, uintptr_t value)
{
	if(type < 0) throw Exception(E_ARGUMENTOUTOFRANGE, "type");
	m_auxv.push_back(auxvec_t(type, value));
}

//-----------------------------------------------------------------------------
// ElfArguments::AppendAuxiliaryVector
//
// Appends an auxiliary vector to the contained list
//
// Arguments:
//
//	type		- Auxiliary vector type code
//	value		- Auxiliary vector value

void ElfArguments::AppendAuxiliaryVector(int type, const uapi::char_t* value)
{
	if(type < 0) throw Exception(E_ARGUMENTOUTOFRANGE, "type");

	// If the value will be pushed into the info block, indicate that with a 
	// negative type, this will be removed during generation
	if(value) m_auxv.push_back(auxvec_t(-type, AppendInfo(value, strlen(value) + 1)));
	else m_auxv.push_back(auxvec_t(type));
}

//-----------------------------------------------------------------------------
// ElfArguments::AppendAuxiliaryVector
//
// Appends an auxiliary vector to the contained list
//
// Arguments:
//
//	type		- Auxiliary vector type code
//	buffer		- Buffer holding binary auxiliary vector data
//	length		- Length of the input buffer

void ElfArguments::AppendAuxiliaryVector(int type, const void* buffer, size_t length)
{
	if(type < 0) throw Exception(E_ARGUMENTOUTOFRANGE, "type");

	// If the value will be pushed into the info block, indicate that with a 
	// negative type, this will be removed during generation
	if(buffer && length) m_auxv.push_back(auxvec_t(-type, AppendInfo(buffer, length)));
	else m_auxv.push_back(auxvec_t(type));
}

//-----------------------------------------------------------------------------
// ElfArguments::AppendEnvironmentVariable
//
// Appends a preformatted environment variable to the contained list
//
// Arguments:
//
//	variable	- Preformatted environment variable in KEY=VALUE format

void ElfArguments::AppendEnvironmentVariable(const uapi::char_t* keyandvalue)
{
	if(!keyandvalue) throw Exception(E_ARGUMENTNULL, "keyandvalue");
	m_envp.push_back(AppendInfo(keyandvalue, strlen(keyandvalue) + 1));
}

//-----------------------------------------------------------------------------
// ElfArguments::AppendEnvironmentVariable
//
// Appends an environment variable to the contained list
//
// Arguments:
//
//	key			- Environment variable key
//	value		- Environment variable value

void ElfArguments::AppendEnvironmentVariable(const uapi::char_t* key, const uapi::char_t* value)
{
	if(!key) throw Exception(E_ARGUMENTNULL, "key");
	if(!value) value = "\0";

	// Append the key, an equal sign and the value to the information block
	uint32_t offset = AppendInfo(key, strlen(key));
	AppendInfo("=", 1);
	AppendInfo(value, strlen(value) + 1);
	
	m_envp.push_back(offset);
}

//-----------------------------------------------------------------------------
// ElfArguments::AppendInfo (private)
//
// Appends data to the information block
//
// Arguments:
//
//	buffer		- Buffer with data to be appended or NULL to reserve
//	length		- Length of the data to be appended

uint32_t ElfArguments::AppendInfo(const void* buffer, size_t length)
{
	// Offsets are cast into unsigned 32-bit integers, the max must be smaller
	static_assert(MAX_INFO_BUFFER < UINT32_MAX, "ElfArguments::MAX_INFO_BUFFER must not be >= UINT32_MAX");

	size_t offset = m_info.size();		// Get current end as the offset

	// Use a const byte pointer and the range-insert method to copy
	const uint8_t* pointer = reinterpret_cast<const uint8_t*>(buffer);
	m_info.insert(m_info.end(), pointer, pointer + length);

	// There is a limit to how big this buffer can be
	if(m_info.size() > MAX_INFO_BUFFER) throw Exception(E_ELFARGUMENTSTOOBIG, static_cast<uint32_t>(m_info.size()), MAX_INFO_BUFFER);

	return static_cast<uint32_t>(offset);
}

//-----------------------------------------------------------------------------
// ElfArguments::GenerateProcessStack
//
// Generates a process stack from the collected ELF arguments
//
// Arguments:
//
//	process		- Handle to the target process
//	base		- Base address of the process stack
//	length		- Length of the process stack

template <ElfClass _elfclass>
void* ElfArguments::GenerateProcessStack(HANDLE process, void* base, size_t length)
{
	using elf = elf_traits<_elfclass>;

	size_t				infolen;				// Length of the information block
	size_t				stacklen;				// Length of the entire stack

	infolen = stacklen = align::up(m_info.size(), 16);

	stacklen += sizeof(typename elf::addr_t);							// argc
	stacklen += sizeof(typename elf::addr_t) * (m_argv.size() + 1);		// argv + NULL
	stacklen += sizeof(typename elf::addr_t) * (m_envp.size() + 1);		// envp + NULL
	stacklen += sizeof(typename elf::auxv_t) * (m_auxv.size() + 1);		// auxv + AT_NULL
	stacklen += sizeof(typename elf::addr_t);							// NULL
	stacklen = align::up(stacklen, 16);									// alignment

	// Make sure that the arguments will fit in the allocated stack space for the process
	_ASSERTE(stacklen <= length);
	if(stacklen > length) throw Exception(E_ELFARGUMENTSEXCEEDSTACK, stacklen, length);

	// Calculate the stack pointer and the address of the information block
	void* stackpointer = reinterpret_cast<void*>((uintptr_t(base) + length) - stacklen);
	typename elf::addr_t infoptr = static_cast<elf::addr_t>((uintptr_t(base) + length) - infolen);

	// Use a heap buffer to collect the information so only one call to WriteProcessMemory is needed
	HeapBuffer<uint8_t> stackimage(stacklen);
	memset(stackimage, 0, stackimage.Size);

	// ARGC
	uint8_t* next = BufferWrite<typename elf::addr_t>(stackimage, static_cast<typename elf::addr_t>(m_argv.size()));

	// ARGV + NULL
	for_each(m_argv.begin(), m_argv.end(), [&](uint32_t& offset) { next = BufferWrite<typename elf::addr_t>(next, infoptr + offset); });
	next = BufferWrite<typename elf::addr_t>(next, 0);

	// ENVP + NULL
	for_each(m_envp.begin(), m_envp.end(), [&](uint32_t& offset) { next = BufferWrite<typename elf::addr_t>(next, infoptr + offset); });
	next = BufferWrite<typename elf::addr_t>(next, 0);

	// AUXV
	for_each(m_auxv.begin(), m_auxv.end(), [&](auxvec_t auxv) {

		// Cast the stored AT_ type code back into a non-negative addr_t
		typename elf::addr_t type = static_cast<typename elf::addr_t>(abs(auxv.a_type));

		// If a_type is positive, this is a straight value, otherwise it's an offset into the information block
		if(auxv.a_type >= 0) next = BufferWrite<typename elf::auxv_t>(next, { type, static_cast<typename elf::addr_t>(auxv.a_val) });
		else next = BufferWrite<typename elf::auxv_t>(next, { type, infoptr + static_cast<typename elf::addr_t>(auxv.a_val) });
	});

	// AT_NULL + TERMINATOR
	next = BufferWrite<typename elf::auxv_t>(next, { LINUX_AT_NULL, 0 });
	next = BufferWrite<typename elf::addr_t>(next, 0);
	
	// INFORMATION BLOCK
	if(m_info.size()) memcpy(&stackimage[stackimage.Size - infolen], m_info.data(), m_info.size());

	// Copy the generated stack image from the local heap buffer into the target process' memory
	try { if(!WriteProcessMemory(process, stackpointer, stackimage, stackimage.Size, nullptr)) throw Win32Exception(); }
	catch(Exception& ex) { throw Exception(E_ELFWRITEARGUMENTS, ex); }

	return stackpointer;				// Return the process stack pointer
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
