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
	while(argv) { if(*argv) AppendArgument(*argv); ++argv; }
	while(envp) { if(*envp) AppendEnvironmentVariable(*envp); ++envp; }
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

inline size_t ElfArguments::AlignUp(size_t offset, size_t alignment)
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

void ElfArguments::AppendEnvironmentVariable(const uapi::char_t* key, const uapi::char_t* value)
{
	if(!key) throw Exception(E_ARGUMENTNULL, "key");
	if(!value) value = "\0";

	// Append the key, an equal sign and the value to the information block
	uintptr_t offset = AppendInfo(key, strlen(key));
	AppendInfo("=", 1);
	AppendInfo(value, strlen(value) + 1);
	
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

uintptr_t ElfArguments::AppendInfo(const void* buffer, size_t length)
{
	size_t offset = m_info.size();		// Get current end as the offset

	// Use a const byte pointer and the range-insert method to copy
	const uint8_t* pointer = reinterpret_cast<const uint8_t*>(buffer);
	m_info.insert(m_info.end(), pointer, pointer + length);

	return offset;
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

//template <ElfClass _class>
//auto ElfArguments<_class>::GenerateMemoryImage(allocator_func allocator, writer_func writer) -> MemoryImage
//{
//	zero_init<MemoryImage>			image;				// Resultant memory image data
//	size_t							stackoffset;		// Offset into memory image of the stack
//
//	if(!allocator) throw Exception(E_ARGUMENTNULL, "allocator");
//	if(!writer) throw Exception(E_ARGUMENTNULL, "writer");
//
//	// Align the information block to 16 bytes; this becomes the start of the stack image
//	stackoffset = AlignUp(m_info.size(), 16);
//
//	// Calculate the additional size required to hold the vectors
//	image.AllocationLength = stackoffset;
//	image.AllocationLength += sizeof(typename elf::addr_t);								// argc
//	image.AllocationLength += sizeof(typename elf::addr_t) * (m_argv.size() + 1);			// argv + NULL
//	image.AllocationLength += sizeof(typename elf::addr_t) * (m_env.size() + 1);			// envp + NULL
//	image.AllocationLength += sizeof(typename elf::auxv_t) * (m_auxv.size() + 1);			// auxv + AT_NULL
//	image.AllocationLength += sizeof(typename elf::addr_t);								// NULL
//	image.AllocationLength = AlignUp(image.AllocationLength, 16);			// alignment
//
//	// Allocate the entire memory image at once using the provided function; it is
//	// expected to be zero initialized after allocation
//	image.AllocationBase = allocator(image.AllocationLength);
//	if(!image.AllocationBase) throw Exception(E_OUTOFMEMORY);
//
//	// Initialize the pointer to and length of the stack image portion for the caller
//	image.StackImage = reinterpret_cast<uint8_t*>(image.AllocationBase) + stackoffset;
//	image.StackImageLength = image.AllocationLength - stackoffset;
//
//	// Use a local HeapBuffer to expedite copying all of the data into the stack image at once
//	HeapBuffer<uint8_t> stackimage(image.StackImageLength);
//	memset(&stackimage, 0, stackimage.Size);
//
//	// a StreamWriter class would be nice here
//
//	stackoffset = 0;
//
//	//// ARGC
//	//stackimage.Cast<addr_t>(stackoffset) = m_argv.size();
//	//stackoffset += sizeof(addr_t);
//
//	//// ARGV
//	//for_each(m_argv.begin(), m_argv.end(), [&](addr_t& offset) 
//	//{ 
//	//	stackimage.Cast<addr_t>(stackoffset) = image.AllocationBase + offset;
//	//	stackoffset += sizeof(addr_t);
//	//});
//	//stackoffset += sizeof(addr_t);		// null
//
//	//// ENVP
//	//for_each(m_env.begin(), m_env.end(), [&](addr_t& offset) 
//	//{ 
//	//	stackimage.Cast<addr_t>(stackoffset) = image.AllocationBase + offset;
//	//	stackoffset += sizeof(addr_t);
//	//});
//	//stackoffset += sizeof(addr_t);		// null
//
//	// AUXV
//
//	// bah, thinking just replace this entire class with a new one
//
//	// Write the information block and the stack image into the allocated memory
//	writer(m_info.data(), reinterpret_cast<void*>(image.AllocationBase), m_info.size());
//	writer(stackimage, reinterpret_cast<void*>(image.StackImage), stackimage.Size);
//
//	//// envp
//	//for_each(m_env.begin(), m_env.end(), [&](addr_t& addr) { writer(&addr, out + offset, sizeof(addr_t)); offset += sizeof(addr_t); });
//	//offset += sizeof(addr_t);
//
//	//// auxv
//	//for_each(m_auxv.begin(), m_auxv.end(), [&](auxv_t& auxv) { writer(&auxv, out + offset, sizeof(auxv_t)); offset += sizeof(auxv_t); });
//	//offset += sizeof(auxv_t);
//
//	//offset += sizeof(addr_t);
//	//offset = AlignUp(offset, 16);
//
//	//_ASSERTE(length == offset);
//
//	//(begin);
//
//	return image;
//}

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
