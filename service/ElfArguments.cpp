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
#include "ElfArguments.h"

#include "Host.h"

#pragma warning(push, 4)

// Explicit Instantiations
//
template const void* ElfArguments::WriteStack<Architecture::x86>(const std::unique_ptr<Host>&, const void*);
#ifdef _M_X64
template const void* ElfArguments::WriteStack<Architecture::x86_64>(const std::unique_ptr<Host>&, const void*);
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

//---------------------------------------------------------------------------
// ElfArguments Constructor
//
// Arguments:
//
//	argc		- Collection of initial command line arguments
//	envp		- Collection of initial environment variables

ElfArguments::ElfArguments(const std::vector<std::string>& argv, const std::vector<std::string>& envp)
{
	for(const auto& iterator : argv) AppendArgument(iterator.c_str());
	for(const auto& iterator : envp) AppendEnvironmentVariable(iterator.c_str());
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
// ElfArguments::WriteStack
//
// Writes the collected ELF arguments to a process stack
//
// Arguments:
//
//	memory			- ProcessMemory instance
//	stackpointer	- Current stack pointer within the ProcessMemory

template <Architecture architecture>
const void* ElfArguments::WriteStack(const std::unique_ptr<Host>& host, const void* stackpointer)
{
	using elf = elf_traits<architecture>;

	size_t				infolen;				// Length of the information block
	size_t				imagelen;				// Length of the entire stack image

	infolen = imagelen = align::up(m_info.size(), 16);

	imagelen += sizeof(typename elf::addr_t);							// argc
	imagelen += sizeof(typename elf::addr_t) * (m_argv.size() + 1);		// argv + NULL
	imagelen += sizeof(typename elf::addr_t) * (m_envp.size() + 1);		// envp + NULL
	imagelen += sizeof(typename elf::auxv_t) * (m_auxv.size() + 1);		// auxv + AT_NULL
	imagelen += sizeof(typename elf::addr_t);							// NULL
	imagelen = align::up(imagelen, 16);									// alignment

	// Calculate the address of the information block and the new stack pointer
	stackpointer = align::down(stackpointer, 16);
	typename elf::addr_t infoptr = static_cast<elf::addr_t>(uintptr_t(stackpointer) - infolen);
	stackpointer = reinterpret_cast<void*>(uintptr_t(stackpointer) - imagelen);

	// Use a heap buffer to collect the information so only one call to write is needed
	HeapBuffer<uint8_t> stackimage(imagelen);
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

	//
	// TODO: This can be done in 2 calls to write to shrink the HeapBuffer and remove the memcpy above
	//

	// Write the stack image into the process memory at the calcuated address
	size_t written = host->WriteMemory(stackpointer, stackimage, stackimage.Size);
	if(written != stackimage.Size) throw Exception(E_ELFWRITEARGUMENTS);

	return stackpointer;				// Return the new stack pointer
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
