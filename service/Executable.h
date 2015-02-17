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
#include "HeapBuffer.h"
#include "FileSystem.h"
#include "LinuxException.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// Executable
//
// Represents an executable, including the command-line arguments, environment
// variables, and initial root/working directories.  Recursively resolves any
// interpreter scripts that represent the target executable.

class Executable
{
public:

	// Destructor
	//
	~Executable()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// FromFile (static)
	//
	// Creates an executable instance from a file system path
	static std::unique_ptr<Executable> FromFile(const std::shared_ptr<VirtualMachine>& vm, const char_t* filename, const char_t* const* arguments,
		const char_t* const* environment, const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir);

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
	Executable(::Architecture architecture, std::shared_ptr<FileSystem::Handle>&& handle, const char_t* const* arguments, const char_t* const* environment,
		const std::shared_ptr<FileSystem::Alias>& rootdir, const std::shared_ptr<FileSystem::Alias>& workingdir);
	friend std::unique_ptr<Executable> std::make_unique<Executable, ::Architecture, std::shared_ptr<FileSystem::Handle>, 
		const char_t* const *&, const char_t* const*&, const std::shared_ptr<FileSystem::Alias>&, const std::shared_ptr<FileSystem::Alias>&>(::Architecture&&, 
		std::shared_ptr<FileSystem::Handle>&&, const char_t* const *& arguments, const char_t* const*& environment, const std::shared_ptr<FileSystem::Alias>& rootdir,
		const std::shared_ptr<FileSystem::Alias>& workingdir);

	//-------------------------------------------------------------------------
	// Member Variables

	const ::Architecture				m_architecture;	// Architecture flag
	std::shared_ptr<FileSystem::Handle>	m_handle;		// File handle
	std::vector<std::string>			m_arguments;	// Command-line arguments
	std::vector<std::string>			m_environment;	// Environment variables
	std::shared_ptr<FileSystem::Alias>	m_rootdir;		// Root directory
	std::shared_ptr<FileSystem::Alias>	m_workingdir;	// Working directory
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EXECUTABLE_H_
