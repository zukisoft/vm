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

#ifndef __ELFLOADER_H_
#define __ELFLOADER_H_
#pragma once

#include <linux/elf.h>
#include <linux/elf-em.h>
#include "Exception.h"
#include "HeapBuffer.h"
#include "MemoryRegion.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

namespace ElfLoader {

	// Metadata
	//
	// Provides information about an image that has been loaded by ElfLoader<>
	struct Metadata
	{
		void*					BaseAddress;
		void*					ProgramHeaders;
		size_t					NumProgramHeaders;
		void*					EntryPoint;
		std::tstring			Interpreter;
	};

};	// namespace 

//-----------------------------------------------------------------------------
// ElfLoader
//
// Loads an ELF binary image into virtual memory
//
//	elfclass	- Expected ELF binary class value
//	ehdr_t		- ELF header structure type
//	phdr_t		- ELF program header structure type
//	shdr_t		- ELF section header structure type

//template<int elfclass, class ehdr_t, class phdr_t, class shdr_t>
//class ElfLoader
//{
//public:
//
//	//-------------------------------------------------------------------------
//	// Member Functions
//
//	// Load (static)
//	//
//	// Loads the image into memory using the provided callback functions
//	static ElfLoader::Metadata Load(void);
//
////	static std::unique_ptr<ElfImage> Load(ElfImage::read_image_func reader) { return Load(reinterpret_cast<HANDLE>(INVALID_HANDLE_VALUE), reader); }
//	//static std::unique_ptr<ElfImage> Load(HANDLE process, ElfImage::read_image_func reader);
//
//private:
//
//	ElfImageT(const ElfImageT&)=delete;
//	ElfImageT& operator=(const ElfImageT&)=delete;
//
//	// Instance Constructor
//	//
//public:
//	ElfImageT(HANDLE process, ElfImage::read_image_func reader);
//private:
//
//	//using me = ElfImageT<elfclass, ehdr_t, phdr_t, shdr_t>;
//	//friend std::unique_ptr<me> std::make_unique<me>(HANDLE&, read_image_func&);
//
//	//friend std::unique_ptr<ElfImageT> std::make_unique<ElfImageT<2,uapi::Elf64_Ehdr,uapi::Elf64_Phdr,uapi::Elf64_Shdr>,HANDLE&,ElfImage::read_image_func&>(HANDLE &,ElfImage::read_image_func &);
//	//friend std::unique_ptr<ElfImageT> std::make_unique<ElfImageT, HANDLE&, ElfImage::read_image_func&>(HANDLE&, ElfImage::read_image_func&);
//	//friend std::unique_ptr<Host> std::make_unique<Host, PROCESS_INFORMATION&>(PROCESS_INFORMATION&);
//
//	//-------------------------------------------------------------------------
//	// Private Member Functions
//
//	// FlagsToProtection
//	//
//	// Converts the ELF p_flags into VirtualAlloc(Ex) protection flags
//	static DWORD FlagsToProtection(uint32_t flags);
//
//	//-------------------------------------------------------------------------
//	// Member Variables
//
//	std::unique_ptr<MemoryRegion>	m_region;			// Allocated virtual memory
//
//	std::tstring					m_interpreter;		// Program interpreter
//	void*							m_base;				// Loaded image base address
//	void*							m_entry;			// Calculated image entry point
//	const phdr_t*					m_phdrs;			// Program headers (in image)
//	size_t							m_phdrents;			// Program header entries
//};
//
//// ElfImage32
////
//// 32-bit x86 ELF binary image type
//using ElfImage32 = ElfImageT<LINUX_ELFCLASS32, uapi::Elf32_Ehdr, uapi::Elf32_Phdr, uapi::Elf32_Shdr>;
//
//// ElfLoader64
////
//// 64-bit x86_64 ELF binary image type
//#ifdef _M_X64
//using ElfImage64 = ElfImageT<LINUX_ELFCLASS64, uapi::Elf64_Ehdr, uapi::Elf64_Phdr, uapi::Elf64_Shdr>;
//#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFLOADER_H_
