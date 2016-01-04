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

#ifndef __EXECUTABLE_H_
#define __EXECUTABLE_H_
#pragma once

#include <functional>
#include <memory>
#include <vector>
#include "Architecture.h"
#include "ExecutableFormat.h"
#include "FileSystem.h"
#include "ProcessMemory.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

// Forward Declarations
//
class Namespace;

//-----------------------------------------------------------------------------
// Executable
//
// Loads an executable binary image into a Host instance.  Interpreter scripts
// are parsed and resolved to their target interpreter binary instance

class Executable
{
public:

	// Destructor
	//
	virtual ~Executable()=default;

	// Executable::Layout Interface
	//
	// Exposes layout information from a loaded executable image
	struct __declspec(novtable) Layout
	{
		// Architecture
		//
		// Gets the architecture of the loaded executable image
		__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
		virtual enum class Architecture getArchitecture(void) const = 0;

		// BreakAddress
		//
		// Pointer to the initial program break address
		__declspec(property(get=getBreakAddress)) uintptr_t BreakAddress;
		virtual uintptr_t getBreakAddress(void) const = 0;

		// EntryPoint
		//
		// Gets the entry point of the loaded executable image
		__declspec(property(get=getEntryPoint)) uintptr_t EntryPoint;
		virtual uintptr_t getEntryPoint(void) const = 0;

		// StackPointer
		//
		// Gets the stack pointer for the loaded executable image
		__declspec(property(get=getStackPointer)) uintptr_t StackPointer;
		virtual uintptr_t getStackPointer(void) const = 0;
	};

	// Executable::PathResolver
	//
	// Function used to resolve a path during executable processing
	using PathResolver = std::function<std::shared_ptr<FileSystem::Handle>(char_t const* path)>;

	//-------------------------------------------------------------------------
	// Member Functions

	// FromFile (static)
	//
	// Creates a new Executable instance from a file system file
	static std::unique_ptr<Executable> FromFile(PathResolver resolver, char_t const* path, char_t const* const* arguments, char_t const* const* environment);

	// Load
	//
	// Loads the executable into a process
	virtual std::unique_ptr<Executable::Layout> Load(ProcessMemory* mem, size_t stacklength) = 0;

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture flag for the executable
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	virtual enum class Architecture getArchitecture(void) const = 0;

	// Format
	//
	// Gets the binary format of the executable
	__declspec(property(get=getFormat)) enum class ExecutableFormat Format;
	virtual enum class ExecutableFormat getFormat(void) const = 0;

protected:

	// Instance Constructor
	//
	Executable()=default;

private:

	Executable(Executable const &)=delete;
	Executable& operator=(Executable const&)=delete;

	// string_vector_t
	//
	// vector<> of std::string objects
	using string_vector_t = std::vector<std::string>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// FromFile (static)
	//
	// Internal version of FromFile that accepts an intermediate set of arguments and environment variables
	static std::unique_ptr<Executable> FromFile(PathResolver resolver, char_t const* path, string_vector_t&& arguments, 
		string_vector_t&& environment, char_t const* originalpath);

	// FromScriptFile (static)
	//
	// Internal version of FromFile that resolves an interpreter script
	static std::unique_ptr<Executable> FromScriptFile(std::shared_ptr<FileSystem::Handle> handle, size_t offset, PathResolver resolver, 
		string_vector_t&& arguments, string_vector_t&& environment, char_t const* originalpath);

	// StringArrayToVector (static)
	//
	// Converts a null-terminated array of C-style strings into a vector<string>
	static string_vector_t StringArrayToVector(char_t const* const* strings);
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __EXECUTABLE_H_
