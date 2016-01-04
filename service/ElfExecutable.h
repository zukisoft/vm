//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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

#ifndef __ELFEXECUTABLE_H_
#define __ELFEXECUTABLE_H_
#pragma once

#include <memory>
#include <vector>
#include "Architecture.h"
#include "Executable.h"
#include "ExecutableFormat.h"
#include "FileSystem.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

// Forward Declarations
//
class Host;

//-----------------------------------------------------------------------------
// ElfExecutable
//
// Specialization of Executable for ELF images
//
// ELF arguments on the x86/x64 platform are provided by pushing a vector of
// values/pointers onto the stack prior to jumping to the entry point:
//
//  STACK POINTER --->  argc          number of arguments
//                      argv[0-n]     pointers to command line arguments
//                      NULL          separator
//                      env[0-n]      pointers to environment variables
//                      NULL          separator
//                      auxv[0-n]     auxiliary vectors
//                      AT_NULL       separator
//                      zero[0-15]    16-byte alignment
//  INFO BLOCK ------>  [argv]        packed command line argument strings
//                      [env]         packed environment variable strings
//                      [auxv]        packed auxiliary vector data
//  STACK BOTTOM ---->  NULL          terminator

class ElfExecutable : public Executable
{
public:

	// Destructor
	//
	virtual ~ElfExecutable()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// FromHandle (static)
	//
	// Creates an ElfExecutable instance from an open file handle
	static std::unique_ptr<ElfExecutable> FromHandle(std::shared_ptr<FileSystem::Handle> handle, PathResolver resolver, std::vector<std::string>&& arguments, 
		std::vector<std::string>&& environment, char_t const* originalpath);

	//-------------------------------------------------------------------------
	// Executable Implementation

	// Load
	//
	// Loads the executable into a process
	virtual std::unique_ptr<Executable::Layout> Load(ProcessMemory* mem, size_t stacklength);

	// getArchitecture
	//
	// Gets the architecture flag for the executable
	virtual enum class Architecture getArchitecture(void) const;

	// getFormat
	//
	// Gets the binary format of the executable
	virtual enum class ExecutableFormat getFormat(void) const;

private:

	ElfExecutable(ElfExecutable const&)=delete;
	ElfExecutable& operator=(ElfExecutable const&)=delete;

	// format_traits_t
	//
	// Architecture specific ELF format traits
	template<enum class Architecture architecture> struct format_traits_t {};

	// fshandle_t
	//
	// FileSystem::Handle shared pointer
	using fshandle_t = std::shared_ptr<FileSystem::Handle>;

	// headerblob_t
	//
	// Dynamically allocated byte array containing the ELF headers
	using headerblob_t = std::unique_ptr<uint8_t[]>;

	// imagelayout_t
	//
	// Layout information for a loaded ELF image
	struct imagelayout_t
	{
		uintptr_t	baseaddress = 0;
		uintptr_t	breakaddress = 0;
		uintptr_t	entrypoint = 0;
		uintptr_t	progheaders = 0;
		size_t		numprogheaders = 0;
	};

	// imagetype
	//
	// Enumeration used to indicate what type of image this is
	enum class imagetype
	{
		primary			= 0,
		interpreter		= 1,
	};

	// stacklayout_t
	//
	// Layout information for a created ELF stack
	struct stacklayout_t
	{
		uintptr_t	baseaddress = 0;
		size_t		length = 0;
		uintptr_t	stackpointer = 0;
	};

	// stringvector_t
	//
	// std::vector<> of ANSI/UTF-8 string objects
	using stringvector_t = std::vector<std::string>;

	// ElfExecutable::Layout
	//
	class Layout : public Executable::Layout
	{
	public:

		// Instance Constructor
		//
		Layout(enum class Architecture architecture, imagelayout_t&& layout, stacklayout_t&& stacklayout);

		//-------------------------------------------------------------------------
		// Executable::Layout Implementation

		// Architecture
		//
		// Gets the architecture of the loaded executable image
		virtual enum class Architecture getArchitecture(void) const;

		// BreakAddress
		//
		// Pointer to the initial program break address
		virtual uintptr_t getBreakAddress(void) const;

		// EntryPoint
		//
		// Gets the entry point of the loaded executable image
		virtual uintptr_t getEntryPoint(void) const;

		// StackPointer
		//
		// Gets the stack pointer for the loaded executable image
		virtual uintptr_t getStackPointer(void) const;

	private:

		Layout(Layout const&)=delete;
		Layout& operator=(Layout const&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		enum class Architecture const	m_architecture;		// Layout architecture
		imagelayout_t const				m_layout;			// Layout information
		stacklayout_t const				m_stacklayout;		// Stack layout information
	};

	// Instance Constructor
	//
	ElfExecutable(enum class Architecture architecture, fshandle_t handle, fshandle_t interpreter, char_t const* originalpath, headerblob_t&& headers,
		stringvector_t&& arguments, stringvector_t&& environment);
	friend std::unique_ptr<ElfExecutable> std::make_unique<ElfExecutable, enum class Architecture, fshandle_t, fshandle_t, char_t const*&, headerblob_t,
		stringvector_t, stringvector_t>(enum class Architecture&&, fshandle_t&&, fshandle_t&&, char_t const*&, headerblob_t&&, stringvector_t&&, stringvector_t&&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// CreateStack<Architecture>
	//
	// Creates the stack image for the executable
	template<enum class Architecture architecture>
	stacklayout_t CreateStack(ProcessMemory* mem, size_t length, imagelayout_t const& primarylayout, imagelayout_t const& interpreterlayout) const;

	// FromHandle<Architecture> (static)
	//
	// Creates an ElfExecutable instance from an open file handle
	template<enum class Architecture architecture>
	static std::unique_ptr<ElfExecutable> FromHandle(fshandle_t handle, PathResolver resolver, stringvector_t&& arguments, stringvector_t&& environment, 
		char_t const* originalpath);

	// Load<Architecture>
	//
	// Architecture-specific implementation of Load
	template<enum class Architecture architecture>
	std::unique_ptr<Executable::Layout> Load(ProcessMemory* mem, size_t stacklength);

	// LoadImage<Architecture> (static)
	//
	// Loads an image into a ProcessMemory implementation
	template<enum class Architecture architecture>
	static imagelayout_t LoadImage(imagetype type, void const* headers, fshandle_t handle, ProcessMemory* mem);
	
	// ReadHeaders<Architecture> (static)
	//
	// Reads ELF and program headers from the provided image file
	template<enum class Architecture architecture>
	static headerblob_t ReadHeaders(fshandle_t handle);

	// ReadInterpreterPath
	//
	// Reads the interpreter (dynamic linker) path from an ELF image file
	template<enum class Architecture architecture>
	static std::string ReadInterpreterPath(void const* headers, fshandle_t handle);

	//-------------------------------------------------------------------------
	// Member Variables

	enum class Architecture const		m_architecture;		// Architecture flag
	fshandle_t const					m_handle;			// Primary executable handle
	fshandle_t const					m_interpreter;		// Interpreter binary handle
	std::string const					m_originalpath;		// Originally specified path
	headerblob_t const					m_headers;			// Binary file headers
	stringvector_t const				m_arguments;		// Command line arguments
	stringvector_t const				m_environment;		// Environment variables
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFEXECUTABLE_H_
