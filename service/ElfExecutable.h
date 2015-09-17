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

	//// CreateElfStack
	////
	//// Architecture-specific implementation of CreateElfStack
	//template<enum class Architecture architecture>
	//friend std::unique_ptr<Executable::StackLayout> CreateElfStack(Host* host, size_t length, ElfExecutable const* executable,
	//	Executable::ImageLayout const* layout);

	//// LoadElfBinary
	////
	//// Architecture-specific implementation of LoadImage
	//template<enum class Architecture architecture>
	//friend std::unique_ptr<Executable::ImageLayout> LoadElfBinary(Host* host, ElfExecutable const* executable);

	//// ReadElfHeaders
	////
	//// Reads the ELF headers from the provided file handle
	//template<enum class Architecture architecture>
	//friend std::unique_ptr<uint8_t[]> ReadElfHeaders(std::shared_ptr<FileSystem::Handle> handle);

	//// ReadElfInterpreterPath
	////
	//// Reads the interpreter (dynamic linker) path from an ELF binary image
	//template<enum class Architecture architecture>
	//friend std::string ReadElfInterpreterPath(void const* headers, std::shared_ptr<FileSystem::Handle> handle);

	//-------------------------------------------------------------------------
	// Member Functions

	// FromHandle (static)
	//
	// Creates an ElfExecutable instance from an open file handle
	static std::unique_ptr<ElfExecutable> FromHandle(std::shared_ptr<FileSystem::Handle> handle, PathResolver resolver, std::vector<std::string>&& arguments, 
		std::vector<std::string>&& environment, char_t const* originalpath);

	//-------------------------------------------------------------------------
	// Executable Implementation

	// getArchitecture
	//
	// Gets the architecture flag for the executable
	virtual enum class Architecture getArchitecture(void) const;

	// getFormat
	//
	// Gets the binary format of the executable
	virtual enum class ExecutableFormat getFormat(void) const;

	// getInterpreter
	//
	// Gets the path to the interpreter (dynamic linker), if present
	virtual std::string getInterpreter(void) const = 0;

private:

	ElfExecutable(ElfExecutable const&)=delete;
	ElfExecutable& operator=(ElfExecutable const&)=delete;

	// format_traits_t
	//
	// Architecture specific ELF format traits
	template<enum class Architecture architecture> struct format_traits_t {};

	// Instance Constructor
	//
	ElfExecutable(enum class Architecture architecture, std::string&& interpreter);
	friend std::unique_ptr<ElfExecutable> std::make_unique<ElfExecutable, enum class Architecture, std::string>(enum class Architecture&&, std::string&&);

	//-------------------------------------------------------------------------
	// Member Variables

	enum class Architecture const	m_architecture;		// Architecture flag
	std::string	const				m_interpreter;		// Path to the dynamic linker
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ELFEXECUTABLE_H_
