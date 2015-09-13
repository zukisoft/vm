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
#include "Architecture.h"
#include "BinaryFormat.h"
#include "FileSystem.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

// Forward Declarations
//
class Namespace;

//-----------------------------------------------------------------------------
// Executable
//
// Resolves an executable file and provides information about the architecture
// and binary format of the target.  Interpreter scripts are parsed and resolved
// to the target binary, modifying the command line argument array appropriately.

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
	// Creates a new Executable instance from an existing file
	static std::unique_ptr<Executable> FromFile(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
		std::shared_ptr<FileSystem::Path> current, char_t const* path);

	// FromFile (static)
	//
	// Creates a new Executable instance from an existing file
	static std::unique_ptr<Executable> FromFile(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
		std::shared_ptr<FileSystem::Path> current, char_t const* path, char_t const* const* arguments, char_t const* const* environment);

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture flag for the executable
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	enum class Architecture getArchitecture(void) const;

	// Arguments
	//
	// Gets a reference to the contained arguments vector
	__declspec(property(get=getArguments)) std::vector<std::string> const& Arguments;
	std::vector<std::string> const& getArguments(void) const;

	// EnvironmentVariables
	//
	// Gets a reference to the contained environment variables vector
	__declspec(property(get=getEnvironmentVariables)) std::vector<std::string> const& EnvironmentVariables;
	std::vector<std::string> const& getEnvironmentVariables(void) const;

	// Format
	//
	// Gets the binary format flag for the executable
	__declspec(property(get=getFormat)) enum class BinaryFormat Format;
	enum class BinaryFormat getFormat(void) const;

	// Handle
	//
	// Gets the FileSystem handle to the binary
	__declspec(property(get=getHandle)) std::shared_ptr<FileSystem::Handle> Handle;
	std::shared_ptr<FileSystem::Handle> getHandle(void) const;

	// OriginalPath
	//
	// Gets the originally specified path of the executable
	__declspec(property(get=getOriginalPath)) char_t const* OriginalPath;
	char_t const* getOriginalPath(void) const;

	// References
	//
	// Gets a reference to the contained executable references vector
	__declspec(property(get=getReferences)) std::vector<std::string> const& References;
	std::vector<std::string> const& getReferences(void) const;

private:

	Executable(Executable const &)=delete;
	Executable& operator=(Executable const&)=delete;

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
	using namespace_t = std::shared_ptr<Namespace>;
	
	// string_vector_t
	//
	// vector<> of std::string objects
	using string_vector_t = std::vector<std::string>;

	// Instance Constructor
	//
	Executable(enum class Architecture architecture, enum class BinaryFormat format, char_t const* originalpath, fshandle_t handle,
		string_vector_t&& references, string_vector_t&& arguments, string_vector_t&& environment);
	friend std::unique_ptr<Executable> std::make_unique<Executable, enum class Architecture, enum class BinaryFormat, char_t const*&, 
		fshandle_t, string_vector_t, string_vector_t, string_vector_t>(enum class Architecture&&, enum class BinaryFormat&&, char_t const*&, fshandle_t&&,
		string_vector_t&&, string_vector_t&&, string_vector_t&&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// FromFile (static)
	//
	// Internal version of FromFile that accepts an intermediate set of arguments
	static std::unique_ptr<Executable> FromFile(namespace_t ns, fspath_t root, fspath_t current, char_t const* originalpath, 
		char_t const* path, string_vector_t&& arguments, string_vector_t&& environment);

	// FromScript (static)
	//
	// Internal version of FromFile that resolves via an interpreter script
	static std::unique_ptr<Executable> FromScript(namespace_t ns, fspath_t root, fspath_t current, char_t const* originalpath,
		fshandle_t scripthandle, size_t dataoffset, string_vector_t&& arguments, string_vector_t&& environment);

	// StringArrayToVector (static)
	//
	// Converts a null-terminated array of C-style strings into a vector<string>
	static string_vector_t StringArrayToVector(char_t const* const* strings);

	//-------------------------------------------------------------------------
	// Member Variables

	enum class Architecture const	m_architecture;		// Architecture flag
	enum class BinaryFormat const	m_format;			// Binary file format
	std::string const				m_originalpath;		// Originally specified path
	fshandle_t const				m_handle;			// Binary file handle
	string_vector_t const			m_references;		// Executable references
	string_vector_t const			m_arguments;		// Command line arguments
	string_vector_t const			m_environment;		// Environment variables
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EXECUTABLE_H_
