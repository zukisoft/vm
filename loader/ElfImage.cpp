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
#include "ElfImage.h"					// Include ELFImage declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Explicit Instantiations

#ifdef _M_X64
template ElfImageT<Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr, Elf64_Sym>;
#else
template ElfImageT<Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr, Elf32_Sym>;
#endif

//-----------------------------------------------------------------------------
// Function Prototypes

extern "C" void __stdcall ElfEntry(void* address, const void* args, size_t argslen);

//-----------------------------------------------------------------------------
// ElfImageT Constructor (private)
//
// Arguments:
//
//	base		- Pointer to the base of the ELF Iimage
//	length		- Optional length to use from mapping

template <class ehdr_t, class phdr_t, class shdr_t, class symb_t>
ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>::ElfImageT(const void* base, size_t length) : 
	m_base(nullptr), m_tlsbase(nullptr), m_tlslength(0), m_entry(nullptr), m_phdrs(nullptr), m_phdrents(0), m_symbols(false)
{
	if(!base) throw Exception(E_ARGUMENTNULL, _T("base"));

	// Validate the header again for good measure and get base pointer for arithmetic
	const ehdr_t* elfheader = ValidateHeader(base, length);
	uintptr_t baseptr = uintptr_t(base);

	// Make an initial pass over the program headers to determine the memory footprint
	// and to look for the presence of a dynamic linker binary path
	uintptr_t minvaddr = UINTPTR_MAX, maxvaddr = 0;
	for(int index = 0; index < elfheader->e_phnum; index++) {

		// Get a pointer to the program header and verify that it won't go beyond the end of the data
		uintptr_t offset = uintptr_t(elfheader->e_phoff + (index * elfheader->e_phentsize));
		if(length < (offset + elfheader->e_phentsize)) throw Exception(E_ELFIMAGETRUNCATED);
		const phdr_t* progheader = reinterpret_cast<const phdr_t*>(baseptr + offset);

		// Check for the presence of an interpreter binary path and store it
		if(progheader->p_type == PT_INTERP) {
			
			if(length < (progheader->p_offset + progheader->p_filesz)) throw Exception(E_ELFIMAGETRUNCATED);
			char* interpreter = reinterpret_cast<char*>(baseptr + progheader->p_offset);
			if(interpreter[progheader->p_filesz - 1] != 0) throw Exception(E_INVALIDINTERPRETER);

#ifdef _UNICODE
			// UNICODE - The string needs to be converted from ANSI
			wchar_t* winterpreter = reinterpret_cast<wchar_t*>(_alloca(progheader->p_filesz * sizeof(wchar_t)));
			if(MultiByteToWideChar(CP_UTF8, 0, interpreter, static_cast<int>(progheader->p_filesz), 
				winterpreter, static_cast<int>(progheader->p_filesz)) == 0) throw Exception(E_INVALIDINTERPRETER);
			m_interpreter = winterpreter;
#else
			m_interpreter = interpreter;				// No conversion necessary
#endif
		}

		// Use loadable segment addresses and lengths to determine the memory footprint
		else if((progheader->p_type == PT_LOAD) && (progheader->p_memsz)) {

			// Calculate the minimum and maximum physical addresses of the segment
			// and adjust the overall minimum and maximums accordingly
			minvaddr = min(uintptr_t(progheader->p_vaddr), minvaddr);
			maxvaddr = max(uintptr_t(progheader->p_vaddr + progheader->p_memsz), maxvaddr);
		}

		// Check for an executable stack flag; currently not going to support this
		else if(progheader->p_type == PT_GNU_STACK) {

			if(progheader->p_flags & PF_X) throw Exception(E_EXECUTABLESTACKFLAG);
		}

		// Check for a thread local storage segment and store the base address and length
		else if(progheader->p_type == PT_TLS) {

			m_tlsbase = reinterpret_cast<void*>(progheader->p_vaddr);
			m_tlslength = progheader->p_memsz;
		}
	}

	try {

		// ET_EXEC images must be reserved at the proper virtual address; ET_DYN images can go anywhere
		// so reserve them at the highest available virtual address
		if(elfheader->e_type == ET_EXEC) m_region.reset(MemoryRegion::Reserve(reinterpret_cast<void*>(minvaddr), maxvaddr - minvaddr));
		else m_region.reset(MemoryRegion::Reserve(maxvaddr - minvaddr, MEM_TOP_DOWN));

	} catch(Exception& ex) { throw Exception(ex, E_RESERVEIMAGEREGION); }

	// ET_EXEC images are loaded at their virtual address, whereas ET_DYN images need a load delta to work with
	intptr_t vaddrdelta = (elfheader->e_type == ET_EXEC) ? 0 : uintptr_t(m_region->Pointer) - minvaddr;

	// Second pass over the program headers to load, commit and protect the program segments
	for(int index = 0; index < elfheader->e_phnum; index++) {

		// Get a pointer to the program header and verify that it won't go beyond the end of the data
		uintptr_t offset = uintptr_t(elfheader->e_phoff + (index * elfheader->e_phentsize));
		const phdr_t* progheader = reinterpret_cast<const phdr_t*>(baseptr + offset);
		if(length < (offset + elfheader->e_phentsize)) throw Exception(E_ELFIMAGETRUNCATED);

		// PT_PHDR - if it falls within the boundaries of the loadable segments, set this so that
		// it can be added to the ELF arguments as an auxiliary vector.  It would be a simple task
		// to just copy it from the header, but I think this is more how it was intended to be used
		if((progheader->p_type == PT_PHDR) && (progheader->p_vaddr >= minvaddr) && ((progheader->p_vaddr + progheader->p_memsz) <= maxvaddr)) {

			 m_phdrs = reinterpret_cast<const phdr_t*>(uintptr_t(progheader->p_vaddr) + vaddrdelta);
			 m_phdrents = progheader->p_memsz / sizeof(phdr_t);
		}

		// PT_LOAD - only load segments that have a non-zero memory footprint defined
		else if((progheader->p_type == PT_LOAD) && (progheader->p_memsz)) {

			// Get the base address of the loadable segment and commit the virtual memory
			uintptr_t segbase = progheader->p_vaddr + vaddrdelta;
			try { m_region->Commit(reinterpret_cast<void*>(segbase), progheader->p_memsz, PAGE_READWRITE); }
			catch(Exception& ex) { throw Exception(ex, E_COMMITIMAGESEGMENT); }

			// Not all segments contain data that needs to be copied from the source image
			if(progheader->p_filesz) {

				// Ensure that there is enough source data to copy and copy it into the segment region
				if(length < (progheader->p_offset + progheader->p_filesz)) throw Exception(E_ELFIMAGETRUNCATED);
				memcpy(reinterpret_cast<void*>(segbase), reinterpret_cast<void*>(baseptr + progheader->p_offset), progheader->p_filesz);
			}

			// Memory that was not loaded from the ELF image must be initialized to zero
			memset(reinterpret_cast<void*>(segbase + progheader->p_filesz), 0, progheader->p_memsz - progheader->p_filesz);

			// Attempt to apply the proper virtual memory protection flags to the segment
			try { m_region->Protect(reinterpret_cast<void*>(segbase), progheader->p_memsz, FlagsToProtection(progheader->p_flags)); }
			catch(Exception& ex) { throw Exception(ex, E_PROTECTIMAGESEGMENT); }
		}
	}

	// Base address of thread local storage, adjusted for load delta
	if(m_tlsbase != nullptr) m_tlsbase = reinterpret_cast<void*>(intptr_t(m_tlsbase) + vaddrdelta);

	// Base address of the image is the original minimum virtual address, adjusted for load delta
	m_base = reinterpret_cast<void*>(minvaddr + vaddrdelta);

	// Calculate the address of the image entry point, if one has been specified in the header
	m_entry = (elfheader->e_entry) ? reinterpret_cast<void*>(elfheader->e_entry + vaddrdelta) : nullptr;

	// Pointers to the main string table as well as the symbol table
	const shdr_t* strheader = nullptr;
	const shdr_t* symheader = nullptr;

	// Make a pass over the section headers looking for the string and symbol tables
	for(int index = 0; index < elfheader->e_shnum; index++) {

		// Get a pointer to the section header and verify that it won't go beyond the end of the data
		uintptr_t offset = uintptr_t(elfheader->e_shoff + (index * elfheader->e_shentsize));
		const shdr_t* sectheader = reinterpret_cast<const shdr_t*>(baseptr + offset);
		if(length < (offset + elfheader->e_shentsize)) throw Exception(E_ELFIMAGETRUNCATED);

		// Check to make sure this isn't the section name string table
		if((sectheader->sh_type == SHT_STRTAB) && (index != elfheader->e_shstrndx)) strheader = sectheader;
		else if(sectheader->sh_type == SHT_SYMTAB) symheader = sectheader;
	}

	// If either the string or symbol table couldn't be found, there are no usable symbols to parse
	if(strheader == nullptr || symheader == nullptr) return;

	// A virtual module needs to be registered with the operating system before symbols can be added.
	uint64_t result = SymLoadModuleEx(GetCurrentProcess(), NULL, NULL, NULL, uintptr_t(m_base), maxvaddr - minvaddr, NULL, SLMFLAG_VIRTUAL);
	if(result != uintptr_t(m_base)) return;
	
	// Iterate over all of the symbols in the symbol table
	int numsymbols = symheader->sh_size / symheader->sh_entsize;
	for(int index = 0; index < numsymbols; index++) {

		uintptr_t offset = uintptr_t(symheader->sh_offset + (index * symheader->sh_entsize));
		const symb_t* symbol = reinterpret_cast<const symb_t*>(baseptr + offset);
		if(length < (offset + symheader->sh_entsize)) throw Exception(E_ELFIMAGETRUNCATED);

		if(symbol->st_name == STN_UNDEF) continue;					// Undefined symbol has a zero name index
		if(ELF_ST_TYPE(symbol->st_info) != STT_FUNC) continue;		// Only loading function symbols here

		// Look up the symbol name in the string table
		const char* symname = reinterpret_cast<const char*>(baseptr + strheader->sh_offset + symbol->st_name);
		
		// The symbol value for ET_EXEC and ET_DYN images is a virtual address.  This is more complicated for ET_REL
		// images, but this class currently doesn't support those anyway.  See ELF documentation for more information.
		// Toggle the 'symbols loaded' flag to TRUE if any symbols are successfully loaded
		if(SymAddSymbol(GetCurrentProcess(), uintptr_t(m_base), symname, symbol->st_value + vaddrdelta, symbol->st_size, 0)) m_symbols = true;
	}
}

//-----------------------------------------------------------------------------
// ElfImageT::Execute
//
// Executes the ELF image by jumping to the entry point
//
// Arguments:
//
//	args			- ELF entry point arguments

template <class ehdr_t, class phdr_t, class shdr_t, class symb_t>
void ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>::Execute(ElfArguments& args)
{
	const void*		argvector;					// Arguments vector data
	size_t			argvectorlen;				// Arguments vector length

	// Gotta have an entry point
	if(m_entry == nullptr) throw Exception(E_NULLELFENTRYPOINT);

	// Create an argument vector to push onto the stack
	argvector = args.CreateArgumentVector(&argvectorlen);

	// Ensure the length is aligned to a 16-byte boundary
	if(argvectorlen & 15) throw Exception(E_ARGUMENTVECTORALIGNMENT);

	// Invoke the entry point assembly helper function
	ElfEntry(m_entry, argvector, argvectorlen);
}

//-----------------------------------------------------------------------------
// ElfImageT::FlagsToProtection (private, static)
//
// Converts an ELF program header p_flags into VirtualAlloc() protection flags
//
// Arguments:
//
//	flags		- ELF program header p_flags value

template <class ehdr_t, class phdr_t, class shdr_t, class symb_t>
DWORD ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>::FlagsToProtection(uint32_t flags)
{
	switch(flags) {

		case PF_X:					return PAGE_EXECUTE;
		case PF_W :					return PAGE_READWRITE;
		case PF_R :					return PAGE_READONLY;
		case PF_X | PF_W :			return PAGE_EXECUTE_READWRITE;
		case PF_X | PF_R :			return PAGE_EXECUTE_READ;
		case PF_W | PF_R :			return PAGE_READWRITE;
		case PF_X | PF_W | PF_R :	return PAGE_EXECUTE_READWRITE;
	}

	return PAGE_NOACCESS;
}

//-----------------------------------------------------------------------------
// ElfImageT::FromFile (static)
//
// Parses and loads the specified ELF image into virtual memory
//
// Arguments:
//
//	path			- ELF image file path
//	loadsymbols		- Flag to load basic symbol information from image

template <class ehdr_t, class phdr_t, class shdr_t, class symb_t>
ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>* ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>::FromFile(const tchar_t* path)
{
	if(!path) throw Exception(E_ARGUMENTNULL, _T("path"));

	try {

		// Attempt to open the image file in read-only sequential scan mode
		std::shared_ptr<File> image(File::OpenExisting(path, GENERIC_READ, 0, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN));

		// Create a read-only memory mapping of the image file
		std::shared_ptr<MappedFile> mapping(MappedFile::CreateFromFile(image, PAGE_READONLY));

		// Create a read-only view against the mapped image file
		std::unique_ptr<MappedFileView> view(MappedFileView::Create(mapping, FILE_MAP_READ));

		// Construct a new ElfImage instance from the mapped image file view
		return new ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>(view->Pointer, view->Length);
	
	} catch(Exception& ex) { throw Exception(ex, E_LOADELFIMAGEFAILED, path); }
}

//-----------------------------------------------------------------------------
// ElfImageT::FromResource (static)
//
// Parses and loads the specified ELF image into virtual memory
//
// Arguments:
//
//	module		- Windows module handle for the resource
//	name		- Resource name
//	type		- Resource type

template <class ehdr_t, class phdr_t, class shdr_t, class symb_t>
ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>* ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>::FromResource(HMODULE module, const tchar_t* name, const tchar_t* type)
{
	// Locate the resource in the target module
	HRSRC hrsrc = FindResource(module, name, type);
	if(hrsrc == NULL) throw Win32Exception();

	// The length of the resource should be at least big enough for the ELF header
	size_t length = SizeofResource(module, hrsrc);
	if(length < sizeof(ehdr_t)) throw Exception(E_ELFIMAGETRUNCATED);

	// Load the resource; note that once loaded you don't unload them again
	HGLOBAL hglobal = LoadResource(module, hrsrc);
	if(hglobal == NULL) throw Win32Exception();

	// Construct a new ElfImageT<> instance from the resource base pointer and length
	return new ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>(LockResource(hglobal), length);
}

//-----------------------------------------------------------------------------
// ElfImageT::ValidateHeader (static)
//
// Validates that the provided pointer points to a 64-bit ELF binary header
//
// Arguments:
//
//	base		- Base address of the ELF binary to test
//	length		- Length of the buffer pointed to by base

template <class ehdr_t, class phdr_t, class shdr_t, class symb_t>
const ehdr_t* ElfImageT<ehdr_t, phdr_t, shdr_t, symb_t>::ValidateHeader(const void* base, size_t length)
{
	if(!base) throw Exception(E_ARGUMENTNULL, _T("base"));

	// Check the length and cast out a pointer to the header structure
	if(length < sizeof(ehdr_t)) throw Exception(E_TRUNCATEDELFHEADER);
	const ehdr_t* header = reinterpret_cast<const ehdr_t*>(base);

	// Check the ELF header magic number
	if(memcmp(&header->e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0) throw Exception(E_INVALIDELFMAGIC);

	// Verify the ELF class matches the build configuration (32-bit vs. 64-bit)
	int elfclass = (sizeof(ehdr_t) == sizeof(Elf32_Ehdr)) ? ELFCLASS32 : ELFCLASS64;
	if(header->e_ident[EI_CLASS] != elfclass) throw Exception(E_INVALIDELFCLASS, header->e_ident[EI_CLASS]);

	// Verify the endianness and version of the ELF binary
	if(header->e_ident[EI_DATA] != ELFDATA2LSB) throw Exception(E_INVALIDELFENCODING, header->e_ident[EI_DATA]);
	if(header->e_ident[EI_VERSION] != EV_CURRENT) throw Exception(E_INVALIDELFVERSION, header->e_ident[EI_VERSION]);

	// Only ET_EXEC and ET_DYN images can currently be loaded by this class
	if((header->e_type != ET_EXEC) && (header->e_type != ET_DYN)) throw Exception(E_INVALIDELFTYPE, header->e_type);

	// The machine type must either be x86 (32 bit) or x86-64 (64 bit)
	int elfmachinetype = (sizeof(ehdr_t) == sizeof(Elf32_Ehdr)) ? EM_386 : EM_X86_64;
	if(header->e_machine != elfmachinetype) throw Exception(E_INVALIDELFMACHINETYPE, header->e_machine);

	// Verify that the version code matches the ELF headers used
	if(header->e_version != EV_CURRENT) throw Exception(E_INVALIDELFVERSION, header->e_version);

	// Verify that the length of the header is the same size as the Elfxx_Ehdr struct
	// and that the header entries are at least as big as the known structures
	if(header->e_ehsize != sizeof(ehdr_t)) throw Exception(E_ELFHEADERFORMAT);
	if((header->e_phentsize) && (header->e_phentsize < sizeof(phdr_t))) throw Exception(E_ELFPROGHEADERFORMAT);
	if((header->e_shentsize) && (header->e_shentsize < sizeof(shdr_t))) throw Exception(E_ELFSECTHEADERFORMAT);

	return header;								// Return a convenience pointer
}

//-----------------------------------------------------------------------------

#pragma warning(pop)