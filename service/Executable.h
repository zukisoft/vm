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

#ifndef __EXECUTABLE_H_
#define __EXECUTABLE_H_
#pragma once

#include <memory>
#include <vector>
#include <linux/elf.h>
#include <linux/fs.h>
#include "Architecture.h"
#include "ElfArguments.h"
#include "ElfImage.h"
#include "HeapBuffer.h"
#include "FileSystem.h"
#include "LinuxException.h"
#include "ProcessMemory.h"
#include "Random.h"
#include "SystemInformation.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// Executable
//
// Represents an executable, including the command-line arguments, environment
// variables, and initial root/working directories.  Recursively resolves any
// interpreter scripts that represent the target executable.
//
// todo this needs more words, this has become a rather crazy class to offload all
// this crap from Process

class Executable
{
public:

	// BinaryFormat
	//
	// Defines the binary format of the executable
	enum class BinaryFormat
	{
		ELF			= 0,			// ELF binary format
		//AOut		= 1,			// A.OUT binary format
	};

	// LoadResult
	//
	// Information about a loaded executable image
	struct LoadResult
	{
		const void*	EntryPoint;		// Executable entry point
		const void*	ProgramBreak;	// Program break address (heap)
		const void*	StackPointer;	// Adjusted stack pointer
	};

	// Destructor
	//
	~Executable()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// FromFile (static)
	//
	// Creates an executable instance from a file system path
	static std::unique_ptr<Executable> FromFile(const char_t* filename, const char_t* const* arguments,
		const char_t* const* environment, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir);

	// Load
	//
	// Loads the executable into a process virtual address space and initializes a stack
	LoadResult Load(const std::unique_ptr<ProcessMemory>& memory, const void* stackpointer) const;

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture flag for the executable
	__declspec(property(get=getArchitecture)) ::Architecture Architecture;
	::Architecture getArchitecture(void) const;

	// Argument
	//
	// Accesses a single argument in the contained argument collection
	__declspec(property(get=getArgument)) const char_t* Argument[];
	const char_t* getArgument(int index) const;

	// ArgumentCount
	//
	// Gets the number of command-line argument strings
	__declspec(property(get=getArgumentCount)) size_t ArgumentCount;
	size_t getArgumentCount(void) const;

	// EnvironmentVariable
	//
	// Accesses a single EnvironmentVariable in the contained EnvironmentVariable collection
	__declspec(property(get=getEnvironmentVariable)) const char_t* EnvironmentVariable[];
	const char_t* getEnvironmentVariable(int index) const;

	// EnvironmentVariableCount
	//
	// Gets the number of command-line EnvironmentVariable strings
	__declspec(property(get=getEnvironmentVariableCount)) size_t EnvironmentVariableCount;
	size_t getEnvironmentVariableCount(void) const;

	// FileName
	//
	// Gets the original filename provided for the executable
	__declspec(property(get=getFileName)) const char_t* FileName;
	const char_t* getFileName(void) const;

	// Format
	//
	// Gets the binary format of the exectable
	__declspec(property(get=getFormat)) BinaryFormat Format;
	BinaryFormat getFormat(void) const;

	// Handle
	//
	// Gets a reference to the executable file handle
	__declspec(property(get=getHandle)) std::shared_ptr<FileSystem::Handle> Handle;
	std::shared_ptr<FileSystem::Handle> getHandle(void) const;

	// RootDirectory
	//
	// Gets the root directory alias used to resolve the executable
	__declspec(property(get=getRootDirectory)) std::shared_ptr<FileSystem::Alias> RootDirectory;
	std::shared_ptr<FileSystem::Alias> getRootDirectory(void) const;

	// WorkingDirectory
	//
	// Gets the working directory alias used to resolve the executable
	__declspec(property(get=getWorkingDirectory)) std::shared_ptr<FileSystem::Alias> WorkingDirectory;
	std::shared_ptr<FileSystem::Alias> getWorkingDirectory(void) const;

private:

	Executable(const Executable&)=delete;
	Executable& operator=(const Executable&)=delete;

	// Instance Constructor
	//
	Executable(::Architecture architecture, BinaryFormat format, std::shared_ptr<FileSystem::Handle>&& handle, const char_t* filename, 
		const char_t* const* arguments, const char_t* const* environment, const std::shared_ptr<FileSystem::Alias>& rootdir,  
		const std::shared_ptr<FileSystem::Alias>& workingdir);

	friend std::unique_ptr<Executable> std::make_unique<Executable, ::Architecture, BinaryFormat, std::shared_ptr<FileSystem::Handle>, 
		const char_t*&, const char_t* const *&, const char_t* const*&, const std::shared_ptr<FileSystem::Alias>&, 
		const std::shared_ptr<FileSystem::Alias>&>(::Architecture&&, BinaryFormat&& format, std::shared_ptr<FileSystem::Handle>&&, 
		const char_t*& filename, const char_t* const *& arguments, const char_t* const*& environment, 
		const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// FromFile (static)
	//
	// Creates an executable instance from a file system path
	static std::unique_ptr<Executable> FromFile(const char_t* originalfilename, const char_t* filename, 
		const char_t* const* arguments, const char_t* const* environment, const std::shared_ptr<FileSystem::Alias>& rootdir, 
		const std::shared_ptr<FileSystem::Alias>& workingdir);

	// LoadELF
	//
	// Loads an ELF binary into a process virtual address space
	LoadResult LoadELF(const std::unique_ptr<ProcessMemory>& memory, const void* stackpointer) const;

	// LoadELF<Architecture>
	//
	// Loads an ELF binary into a process virtual address space
	template<::Architecture architecture>
	LoadResult LoadELF(const std::unique_ptr<ProcessMemory>& memory, const void* stackpointer) const;

	//-------------------------------------------------------------------------
	// Member Variables

	const ::Architecture				m_architecture;		// Architecture flag
	const BinaryFormat					m_format;			// Binary file format
	std::shared_ptr<FileSystem::Handle>	m_handle;			// File handle
	std::string							m_filename;			// Original file name
	std::vector<std::string>			m_arguments;		// Command-line arguments
	std::vector<std::string>			m_environment;		// Environment variables
	std::shared_ptr<FileSystem::Alias>	m_rootdir;			// Root directory
	std::shared_ptr<FileSystem::Alias>	m_workingdir;		// Working directory
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EXECUTABLE_H_
