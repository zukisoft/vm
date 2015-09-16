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
#include "ExecutableFormat.h"
#include "FileSystem.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

// Forward Declarations
//
class Host;
class Namespace;

//-----------------------------------------------------------------------------
// Executable
//
// Resolves an executable file and provides information about the architecture
// and binary format of the target.  Interpreter scripts are parsed and resolved
// to the target binary, modifying the command line argument array appropriately

class Executable
{
public:

	// Destructor
	//
	virtual ~Executable()=default;

	// ElfExecutable::Layout
	//
	// Exposes layout information from a loaded executable image
	struct __declspec(novtable) Layout
	{
		//-------------------------------------------------------------------------
		// Properties

		// BaseAddress
		//
		// Gets the base address of the loaded executable image
		__declspec(property(get=getBaseAddress)) void const* BaseAddress;
		virtual void const* getBaseAddress(void) const = 0;

		// BreakAddress
		//
		// Pointer to the initial program break address
		__declspec(property(get=getBreakAddress)) void const* BreakAddress;
		virtual void const* getBreakAddress(void) const = 0;

		// EntryPoint
		//
		// Gets the entry point of the loaded executable image
		__declspec(property(get=getEntryPoint)) void const* EntryPoint;
		virtual void const* getEntryPoint(void) const = 0;
	};

	//-------------------------------------------------------------------------
	// Member Functions

	// CreateStack
	//
	// todo: pure virtual

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

	// Load
	//
	// Loads the executable into a Host instance
	virtual std::unique_ptr<Executable::Layout> Load(Host* host) const = 0;

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture flag for the executable
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	virtual enum class Architecture getArchitecture(void) const = 0;

	// Format
	//
	// Gets the binary format flag for the executable
	__declspec(property(get=getFormat)) enum class ExecutableFormat Format;
	virtual enum class ExecutableFormat getFormat(void) const = 0;

	// Interpreter
	//
	// Gets the path to the interpreter (dynamic linker) or nullptr
	__declspec(property(get=getInterpreter)) char_t const* Interpreter;
	virtual char_t const* getInterpreter(void) const = 0;

protected:

	// Instance Constructor
	//
	Executable()=default;

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
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EXECUTABLE_H_
