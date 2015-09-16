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

#ifndef __ELFEXECUTABLE_H_
#define __ELFEXECUTABLE_H_
#pragma once

#include <memory>
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

class ElfExecutable : public Executable
{
public:

	// Destructor
	//
	virtual ~ElfExecutable()=default;

	//-------------------------------------------------------------------------
	// Friend Functions

	// LoadElfBinary
	//
	// Architecture-specific implementation of Load
	template<enum class Architecture architecture>
	friend std::unique_ptr<Executable::Layout> LoadElfBinary(Host* host, ElfExecutable const* executable);

	// ReadElfHeaders
	//
	// Reads the ELF headers from the provided file handle
	template<enum class Architecture architecture>
	friend std::unique_ptr<uint8_t[]> ReadElfHeaders(std::shared_ptr<FileSystem::Handle> handle);

	// ReadElfInterpreterPath
	//
	// Reads the interpreter (dynamic linker) path from an ELF binary image
	template<enum class Architecture architecture>
	friend std::string ReadElfInterpreterPath(void const* headers, std::shared_ptr<FileSystem::Handle> handle);

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new ElfExecutable instance from an existing executable file handle
	static std::unique_ptr<ElfExecutable> Create(std::shared_ptr<FileSystem::Handle> handle, char_t const* originalpath, 
		std::vector<std::string>&& arguments, std::vector<std::string>&& environment);

	//-------------------------------------------------------------------------
	// Executable Implementation

	// Load
	//
	// Loads the executable into a Host instance
	virtual std::unique_ptr<Executable::Layout> Load(Host* host) const;

	// getArchitecture
	//
	// Gets the architecture flag for the executable
	virtual enum class Architecture getArchitecture(void) const;

	// getFormat
	//
	// Gets the binary format flag for the executable
	virtual enum class ExecutableFormat getFormat(void) const;

	// getInterpreter
	//
	// Gets the path to the interpreter (dynamic linker) or nullptr
	virtual char_t const* getInterpreter(void) const;

private:

	ElfExecutable(ElfExecutable const&)=delete;
	ElfExecutable& operator=(ElfExecutable const&)=delete;

	// blob_t
	//
	// uint8_t[] unique pointer
	using blob_t = std::unique_ptr<uint8_t[]>;

	// format_traits_t
	//
	// Architecture specific ELF format traits
	template<enum class Architecture architecture> struct format_traits_t {};

	// fshandle_t
	//
	// FileSystem::Handle shared pointer
	using fshandle_t = std::shared_ptr<FileSystem::Handle>;
	
	// layout_t
	//
	// Provides metadata about a loaded ELF image
	struct layout_t
	{
		void const*		baseaddress = nullptr;
		void const*		breakaddress = nullptr;
		void const*		entrypoint = nullptr;
		void const*		progheaders = nullptr;
		size_t			numprogheaders = 0;
	};

	// string_vector_t
	//
	// vector<> of std::string objects
	using string_vector_t = std::vector<std::string>;

	// ElfExecutable::Layout
	//
	// Executable::Layout implementation
	class Layout : public Executable::Layout
	{
	public:

		// Instance Constructor
		//
		Layout(layout_t&& layout);

		// Destructor
		//
		virtual ~Layout()=default;

		//-------------------------------------------------------------------------
		// Executable::Layout Implementation

		// getBaseAddress
		//
		// Gets the base address of the loaded executable image
		virtual void const* getBaseAddress(void) const;

		// getBreakAddress
		//
		// Pointer to the initial program break address
		virtual void const* getBreakAddress(void) const;

		// getEntryPoint
		//
		// Gets the entry point of the loaded executable image
		virtual void const* getEntryPoint(void) const;

	private:

		Layout(Layout const&)=delete;
		Layout& operator=(Layout const&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		layout_t const			m_layout;			// Layout information
	};

	// Instance Constructor
	//
	ElfExecutable(enum class Architecture architecture, char_t const* originalpath, fshandle_t handle, blob_t headers, 
		std::string&& interpreter, string_vector_t&& arguments, string_vector_t&& environment);

	friend std::unique_ptr<ElfExecutable> std::make_unique<ElfExecutable, enum class Architecture, char_t const*&, fshandle_t, 
		blob_t, std::string, string_vector_t, string_vector_t>(enum class Architecture&&, char_t const*&, fshandle_t&&, blob_t&&, 
		std::string&&, string_vector_t&&, string_vector_t&&);

	//-------------------------------------------------------------------------
	// Member Variables

	enum class Architecture const		m_architecture;		// Architecture flag
	std::string const					m_originalpath;		// Originally specified path
	fshandle_t const					m_handle;			// Executable file handle
	blob_t const						m_headers;			// Blob with header data
	std::string const					m_interpreter;		// Interpreter binary path
	string_vector_t const				m_arguments;		// Command line arguments
	string_vector_t const				m_environment;		// Environment variables
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFEXECUTABLE_H_
