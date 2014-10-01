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

template ElfArguments<ElfClass::x86>;

#ifdef _M_X64
template ElfArguments<ElfClass::x86_64>;
#endif

//---------------------------------------------------------------------------
// ElfArguments Constructor
//
// Arguments:
//
//	argc		- Pointer to the initial set of command line arguments
//	envp		- Pointer to the initial set of environment variables

template <ElfClass _class>
ElfArguments<_class>::ElfArguments(const uapi::char_t** argv, const uapi::char_t** envp)
{
	// Iterate over all the command line arguments and environment variables and
	// append them as-is into the information block; this will not check for syntax
	// of the environment variables, they are assumed to be valid enough
	while(argv && *argv) { AppendArgument(*argv); ++argv; }
	while(envp && *envp) { AppendEnvironmentVariable(*envp); ++envp; }
}

//---------------------------------------------------------------------------
// ElfArguments::AlignUp (private, static)
//
// Aligns an offset up to the specified alignment
//
// Arguments:
//
//	offset		- Offset to be aligned
//	alignment	- Alignment

template <ElfClass _class>
inline size_t ElfArguments<_class>::AlignUp(size_t offset, size_t alignment)
{
	if(alignment < 1) throw Exception(E_ARGUMENTOUTOFRANGE, _T("alignment"));

	if(offset == 0) return 0;
	else return offset + ((alignment - (offset % alignment)) % alignment);
}

//-----------------------------------------------------------------------------
// ElfArguments::AppendArgument
//
// Appends an argument to the contained argument list
//
// Arguments:
//
//	value		- Command-line argument to be added

template <ElfClass _class>
void ElfArguments<_class>::AppendArgument(const uapi::char_t* value)
{
	if(!value) throw Exception(E_ARGUMENTNULL, _T("value"));
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

template <ElfClass _class>
void ElfArguments<_class>::AppendAuxiliaryVector(typename elf::addr_t type, typename elf::addr_t value)
{
	m_auxv.push_back(typename elf::auxv_t(type, value));
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

template <ElfClass _class>
void ElfArguments<_class>::AppendAuxiliaryVector(typename elf::addr_t type, const uapi::char_t* value)
{
	// If a non-null pointer was provided, append the data to the info block and store the 
	// offset to it in the aux vector value; this will be resolved to a pointer later
	if(value) m_auxv.push_back(typename elf::auxv_t(type | AT_ISOFFSET, static_cast<typename elf::addr_t>(AppendInfo(value, strlen(value) + 1))));
	else m_auxv.push_back(typename elf::auxv_t(type));
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

template <ElfClass _class>
void ElfArguments<_class>::AppendAuxiliaryVector(typename elf::addr_t type, const void* buffer, size_t length)
{
	// If a non-null pointer was provided, append the data to the info block and store the 
	// offset to it in the aux vector value; this will be resolved to a pointer later
	if(buffer) m_auxv.push_back(typename elf::auxv_t(type | AT_ISOFFSET, static_cast<typename elf::addr_t>(AppendInfo(buffer, length))));
	else m_auxv.push_back(typename elf::auxv_t(type));
}

//-----------------------------------------------------------------------------
// ElfArguments::AppendEnvironmentVariable
//
// Appends a preformatted environment variable to the contained list
//
// Arguments:
//
//	variable	- Preformatted environment variable in KEY=VALUE format

template <ElfClass _class>
void ElfArguments<_class>::AppendEnvironmentVariable(const uapi::char_t* keyandvalue)
{
	if(!keyandvalue) throw Exception(E_ARGUMENTNULL, _T("keyandvalue"));
	m_env.push_back(AppendInfo(keyandvalue, strlen(keyandvalue) + 1));
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

template <ElfClass _class>
void ElfArguments<_class>::AppendEnvironmentVariable(const uapi::char_t* key, const uapi::char_t* value)
{
	if(!key) throw Exception(E_ARGUMENTNULL, _T("key"));

	// Append the key and an equal sign to the information block
	size_t offset = AppendInfo(key, strlen(key));
	AppendInfo("=", 1, false);

	// If a value was specifed, append that and a NULL terminator, otherwise
	// just append a NULL terminator after the equal sign
	if(value) AppendInfo(value, strlen(value) + 1, false);
	else AppendInfo("\0", 1, false);
	
	m_env.push_back(offset);
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
//	align		- Flag to align the data (false if appending multiple)

template <ElfClass _class>
size_t ElfArguments<_class>::AppendInfo(const void* buffer, size_t length, bool align)
{
	// Align the data to addr_t size before appending; this also becomes the offset
	size_t offset = (align) ? AlignUp(m_info.size(), sizeof(typename elf::addr_t)) : m_info.size();
	if(offset >= elf::max_addr_t) throw Exception(E_OUTOFMEMORY);

	// If aligning the data buffer, increase the size of the vector to accomodate
	if(align) m_info.resize(offset);

	// Use a const byte pointer and the range-insert method to copy
	const uint8_t* pointer = reinterpret_cast<const uint8_t*>(buffer);
	m_info.insert(m_info.end(), pointer, pointer + length);

	return offset;						// Return offset to the data
}

//-----------------------------------------------------------------------------
// ElfArguments::GenerateMemoryImage
//
// Generates the memory image for the ELF arguments
//
// Arguments:
//
//	allocator	- Function used to allocate the memory required
//	writer		- Function used to write data into the allocated memory

template <ElfClass _class>
auto ElfArguments<_class>::GenerateMemoryImage(allocator_func allocator, writer_func writer) -> MemoryImage
{
	zero_init<MemoryImage>			image;				// Resultant memory image data
	size_t							stackoffset;		// Offset into memory image of the stack

	if(!allocator) throw Exception(E_ARGUMENTNULL, "allocator");
	if(!writer) throw Exception(E_ARGUMENTNULL, "writer");

	// Align the information block to 16 bytes; this becomes the start of the stack image
	stackoffset = AlignUp(m_info.size(), 16);

	// Calculate the additional size required to hold the vectors
	image.AllocationLength = stackoffset;
	image.AllocationLength += sizeof(typename elf::addr_t);								// argc
	image.AllocationLength += sizeof(typename elf::addr_t) * (m_argv.size() + 1);			// argv + NULL
	image.AllocationLength += sizeof(typename elf::addr_t) * (m_env.size() + 1);			// envp + NULL
	image.AllocationLength += sizeof(typename elf::auxv_t) * (m_auxv.size() + 1);			// auxv + AT_NULL
	image.AllocationLength += sizeof(typename elf::addr_t);								// NULL
	image.AllocationLength = AlignUp(image.AllocationLength, 16);			// alignment

	// Allocate the entire memory image at once using the provided function; it is
	// expected to be zero initialized after allocation
	image.AllocationBase = allocator(image.AllocationLength);
	if(!image.AllocationBase) throw Exception(E_OUTOFMEMORY);

	// Initialize the pointer to and length of the stack image portion for the caller
	image.StackImage = reinterpret_cast<uint8_t*>(image.AllocationBase) + stackoffset;
	image.StackImageLength = image.AllocationLength - stackoffset;

	// Use a local HeapBuffer to expedite copying all of the data into the stack image at once
	HeapBuffer<uint8_t> stackimage(image.StackImageLength);
	memset(&stackimage, 0, stackimage.Size);

	// a StreamWriter class would be nice here

	stackoffset = 0;

	//// ARGC
	//stackimage.Cast<addr_t>(stackoffset) = m_argv.size();
	//stackoffset += sizeof(addr_t);

	//// ARGV
	//for_each(m_argv.begin(), m_argv.end(), [&](addr_t& offset) 
	//{ 
	//	stackimage.Cast<addr_t>(stackoffset) = image.AllocationBase + offset;
	//	stackoffset += sizeof(addr_t);
	//});
	//stackoffset += sizeof(addr_t);		// null

	//// ENVP
	//for_each(m_env.begin(), m_env.end(), [&](addr_t& offset) 
	//{ 
	//	stackimage.Cast<addr_t>(stackoffset) = image.AllocationBase + offset;
	//	stackoffset += sizeof(addr_t);
	//});
	//stackoffset += sizeof(addr_t);		// null

	// AUXV

	// bah, thinking just replace this entire class with a new one

	// Write the information block and the stack image into the allocated memory
	writer(m_info.data(), reinterpret_cast<void*>(image.AllocationBase), m_info.size());
	writer(stackimage, reinterpret_cast<void*>(image.StackImage), stackimage.Size);

	//// envp
	//for_each(m_env.begin(), m_env.end(), [&](addr_t& addr) { writer(&addr, out + offset, sizeof(addr_t)); offset += sizeof(addr_t); });
	//offset += sizeof(addr_t);

	//// auxv
	//for_each(m_auxv.begin(), m_auxv.end(), [&](auxv_t& auxv) { writer(&auxv, out + offset, sizeof(auxv_t)); offset += sizeof(auxv_t); });
	//offset += sizeof(auxv_t);

	//offset += sizeof(addr_t);
	//offset = AlignUp(offset, 16);

	//_ASSERTE(length == offset);

	//(begin);

	return image;
}

//const void* ElfArguments<_class>::CreateArgumentVector(size_t* length)
//{
//	(length);
//	return nullptr;
	//if(!length) throw Exception(E_ARGUMENTNULL, _T("length"));

	//// ALIGNMENT
	//m_offset = AlignUp(m_offset, 16);
	//if(m_offset > m_info->Length) throw Exception(E_OUTOFMEMORY);

	//// ARGC / ARGV
	//uintptr_t begin = uintptr_t(AppendInfo(static_cast<addr_t>(m_argv.size())));
	//for_each(m_argv.begin(), m_argv.end(), [&](addr_t& addr) { AppendInfo(addr); });
	//AppendInfo(static_cast<addr_t>(NULL));

	//// ENVIRONMENT VARIABLES
	//for_each(m_env.begin(), m_env.end(), [&](addr_t addr) { AppendInfo(addr); });
	//AppendInfo(static_cast<addr_t>(NULL));

	//// AUXILIARY VECTORS
	//for_each(m_auxv.begin(), m_auxv.end(), [&](auxv_t auxv) { AppendInfo(auxv); });
	//AppendInfo(auxv_t(LINUX_AT_NULL, 0));

	//// TERMINATOR
	//AppendInfo(static_cast<addr_t>(NULL));

	//// ALIGNMENT
	//m_offset = AlignUp(m_offset, 16);
	//if(m_offset > m_info->Length) throw Exception(E_OUTOFMEMORY);

	//// Calculate the end address and the overall length
	//uintptr_t end = uintptr_t(m_info->Pointer) + m_offset;
	//*length = (end - begin);

	//return reinterpret_cast<const void*>(begin);
//}

//-----------------------------------------------------------------------------

#pragma warning(pop)
