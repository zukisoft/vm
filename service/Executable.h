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
#include <string>
#include <vector>
#include "Architecture.h"
#include "FileSystem.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

// Forward Declarations
//
class Namespace;
class ProcessMemory;

//-----------------------------------------------------------------------------
// Executable
//
// Represents an executable, including the command-line arguments, environment
// variables, and initial root/working directories.  Recursively resolves any
// interpreter scripts that represent the target executable

class Executable
{
public:

	// BinaryFormat
	//
	// Defines the binary format of the executable
	enum class BinaryFormat
	{
		ELF			= 0,			// ELF binary format
		//AOut		= 1,			// A.OUT binary format (todo: future)
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
	static std::unique_ptr<Executable> FromFile(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> rootdir, 
		std::shared_ptr<FileSystem::Path> workingdir, const char_t* filename, const char_t* const* arguments, const char_t* const* environment);

	// Load
	//
	// Loads the executable into a process virtual address space and initializes a stack
	LoadResult Load(const std::unique_ptr<ProcessMemory>& memory, const void* stackpointer) const;

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture flag for the executable
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	enum class Architecture getArchitecture(void) const;

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

	// Namespace
	//
	// Gets the Namespace from which the executable was resolved
	__declspec(property(get=getNamespace)) std::shared_ptr<class Namespace> Namespace;
	std::shared_ptr<class Namespace> getNamespace(void) const;

	// RootDirectory
	//
	// Gets the root directory alias used to resolve the executable
	__declspec(property(get=getRootDirectory)) std::shared_ptr<FileSystem::Path> RootDirectory;
	std::shared_ptr<FileSystem::Path> getRootDirectory(void) const;

	// WorkingDirectory
	//
	// Gets the working directory alias used to resolve the executable
	__declspec(property(get=getWorkingDirectory)) std::shared_ptr<FileSystem::Path> WorkingDirectory;
	std::shared_ptr<FileSystem::Path> getWorkingDirectory(void) const;

private:

	Executable(const Executable&)=delete;
	Executable& operator=(const Executable&)=delete;

	// fshandle_t
	//
	// FileSystem::Handle shared pointer
	using fshandle_t = std::shared_ptr<FileSystem::Handle>;

	// fspath_t
	//
	// FileSystem::Path shared pointer
	using fspath_t = std::shared_ptr<FileSystem::Path>;

	// namespace_t
	//
	// Namespace shared pointer
	using namespace_t = std::shared_ptr<class Namespace>;

	// string_vector_t
	//
	// vector<> of string instances
	using string_vector_t = std::vector<std::string>;

	// Instance Constructor
	//
	Executable(enum class Architecture architecture, BinaryFormat format, std::shared_ptr<FileSystem::Handle> handle, const char_t* filename, 
		const char_t* const* arguments, const char_t* const* environment, std::shared_ptr<class Namespace> ns, std::shared_ptr<FileSystem::Path> rootdir,  
		std::shared_ptr<FileSystem::Path> workingdir);

	friend std::unique_ptr<Executable> std::make_unique<Executable, enum class Architecture, BinaryFormat, std::shared_ptr<FileSystem::Handle>, 
		const char_t*&, const char_t* const *&, const char_t* const*&, std::shared_ptr<class Namespace>, std::shared_ptr<FileSystem::Path>, 
		std::shared_ptr<FileSystem::Path>>(enum class Architecture&&, BinaryFormat&& format, std::shared_ptr<FileSystem::Handle>&& handle, 
		const char_t*& filename, const char_t* const *& arguments, const char_t* const*& environment, std::shared_ptr<class Namespace>&& ns,
		std::shared_ptr<FileSystem::Path>&& rootdir, std::shared_ptr<FileSystem::Path>&& workingdir);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// FromFile (static)
	//
	// Creates an executable instance from a file system path
	static std::unique_ptr<Executable> FromFile(std::shared_ptr<class Namespace> ns, std::shared_ptr<FileSystem::Path> rootdir, 
		std::shared_ptr<FileSystem::Path> workingdir, const char_t* originalfilename, const char_t* filename, const char_t* const* arguments, 
		const char_t* const* environment);

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

	const ::Architecture		m_architecture;			// Architecture flag
	const BinaryFormat			m_format;				// Binary file format
	const fshandle_t			m_handle;				// File handle
	const std::string			m_filename;				// Original file name
	string_vector_t				m_arguments;			// Command-line arguments
	string_vector_t				m_environment;			// Environment variables
	const namespace_t			m_ns;					// Namespace instance
	const fspath_t				m_rootdir;				// Root directory
	const fspath_t				m_workingdir;			// Working directory
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EXECUTABLE_H_
