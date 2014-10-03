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
template ElfArguments::StackImage ElfArguments::GenerateStackImage<ElfClass::x86>(HANDLE);
#ifdef _M_X64
template ElfArguments::StackImage ElfArguments::GenerateStackImage<ElfClass::x86_64>(HANDLE);
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
// ElfArguments::GenerateStackImage
//
// Generates the memory image for the collected ELF arguments
//
// Arguments:
//
//	process		- Handle to the target process

template <ElfClass _elfclass>
ElfArguments::StackImage ElfArguments::GenerateStackImage(HANDLE process)
{
	using elf = elf_traits<_elfclass>;

	size_t					imagelen;					// Length of the entire image
	size_t					stackoffset;				// Offset to the stack image

	// Align the information block to 16 bytes; this becomes the start of the stack image
	imagelen = stackoffset = AlignUp(m_info.size(), 16);

	// Calculate the additional size required to hold the vectors
	imagelen += sizeof(typename elf::addr_t);							// argc
	imagelen += sizeof(typename elf::addr_t) * (m_argv.size() + 1);		// argv + NULL
	imagelen += sizeof(typename elf::addr_t) * (m_envp.size() + 1);		// envp + NULL
	imagelen += sizeof(typename elf::auxv_t) * (m_auxv.size() + 1);		// auxv + AT_NULL
	imagelen += sizeof(typename elf::addr_t);							// NULL
	imagelen = AlignUp(imagelen, 16);									// alignment

	// Allocate the memory to hold the arguments in the hosted process
	std::unique_ptr<MemoryRegion> allocation = MemoryRegion::Reserve(process, imagelen, MEM_COMMIT | MEM_TOP_DOWN);

	// If there is any data in the information block, write that into the hosted process
	try { if(m_info.data() && !WriteProcessMemory(process, allocation->Pointer, m_info.data(), m_info.size(), nullptr)) throw Win32Exception(); }
	catch(Exception& ex) { throw Exception(E_ELFWRITEARGUMENTS, ex); }
	
	// Use a local heap buffer to collect all of the stack image data locally before writing it
	HeapBuffer<uint8_t> stackimage(imagelen - stackoffset);
	memset(&stackimage, 0, stackimage.Size);

	// Cast the allocation pointer into an elf::addr_t for simpler arithmetic
	typename elf::addr_t allocptr = reinterpret_cast<typename elf::addr_t>(allocation->Pointer);

	// ARGC
	uint8_t* next = BufferWrite<typename elf::addr_t>(stackimage, static_cast<typename elf::addr_t>(m_argv.size()));

	// ARGV + NULL
	for_each(m_argv.begin(), m_argv.end(), [&](uint32_t& offset) { next = BufferWrite<typename elf::addr_t>(next, allocptr + offset); });
	next = BufferWrite<typename elf::addr_t>(next, 0);

	// ENVP + NULL
	for_each(m_envp.begin(), m_envp.end(), [&](uint32_t& offset) { next = BufferWrite<typename elf::addr_t>(next, allocptr + offset); });
	next = BufferWrite<typename elf::addr_t>(next, 0);

	// AUXV
	for_each(m_auxv.begin(), m_auxv.end(), [&](auxvec_t auxv) {

		// Cast the stored AT_ type code back into a non-negative addr_t
		typename elf::addr_t type = static_cast<typename elf::addr_t>(abs(auxv.a_type));

		// If a_type is positive, this is a straight value, otherwise it's an offset into the information block
		if(auxv.a_type >= 0) next = BufferWrite<typename elf::auxv_t>(next, { type, static_cast<typename elf::addr_t>(auxv.a_val) });
		else next = BufferWrite<typename elf::auxv_t>(next, { type, allocptr + static_cast<typename elf::addr_t>(auxv.a_val) });
	});

	// AT_NULL + TERMINATOR
	next = BufferWrite<typename elf::auxv_t>(next, { LINUX_AT_NULL, 0 });
	next = BufferWrite<typename elf::addr_t>(next, 0);

	// Write the stack image portion of the arguments into the host process address space
	void* pv = reinterpret_cast<uint8_t*>(allocation->Pointer) + stackoffset;
	try { if(!WriteProcessMemory(process, pv, &stackimage, stackimage.Size, nullptr)) throw Win32Exception(); }
	catch(Exception& ex) { throw Exception(E_ELFWRITEARGUMENTS, ex); }

	// Detach the MemoryRegion allocation and return the metadata to the caller
	return StackImage { reinterpret_cast<uint8_t*>(allocation->Detach()) + stackoffset, stackimage.Size };
}

//-----------------------------------------------------------------------------

#pragma warning(pop)