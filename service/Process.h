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

#ifndef __PROCESS_H_
#define __PROCESS_H_
#pragma once

#include <array>
#include <memory>
#include <linux/elf.h>
#include "ElfImage.h"
#include "Exception.h"
#include "HandleStreamReader.h"
#include "HeapBuffer.h"
#include "Host.h"
#include "LinuxException.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// Process
//
// Process represents a single hosted process instance.
//
// TODO: needs lot more words

class Process
{
public:

	// Destructor
	//
	~Process();

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new process instance
	std::unique_ptr<Process> Create(const std::shared_ptr<VirtualMachine>& vm, const uapi::char_t* path,
		const uapi::char_t** arguments, const uapi::char_t** environment);

	// Resume
	//
	// Resumes the process from a suspended state

	// Suspend
	//
	// Suspends the process

private:

	Process(const Process&)=delete;
	Process& operator=(const Process&)=delete;

	// Forward Declarations
	//
	class AuxiliaryVector;

	// Instance Constructor
	//
	Process(std::unique_ptr<Host>&& host) : m_host(std::move(host)) {}
	friend std::unique_ptr<Process> std::make_unique<Process, std::unique_ptr<Host>>(std::unique_ptr<Host>&&);

	//-------------------------------------------------------------------------
	// Private Type Declarations

	typedef union {

		uint8_t	ansi_magic[3];				// 0x23, 0x21, 0x20
		uint8_t	utf8_magic[6];				// 0xEF, 0xBB, 0xBF, 0x23, 0x21, 0x20
		uint8_t	utf16_magic[8];				// 0xFF, 0xFE, 0x23, 0x00, 0x21, 0x00, 0x20, 0x00
		uint8_t	elf_ident[LINUX_EI_NIDENT];	// "\177ELF"
	
	} BinaryMagic;

	class AuxiliaryVector
	{
	public:

		AuxiliaryVector(const uapi::char_t** arguments, const uapi::char_t** environment) { (arguments); (environment); }
		~AuxiliaryVector()=default;

	private:

		AuxiliaryVector(const AuxiliaryVector&)=delete;
		AuxiliaryVector& operator=(const AuxiliaryVector&)=delete;
	};

	enum class Signal
	{
		Ready = 0,
	};

	class Signals
	{
	public:

		// Instance Constructor
		//
		// TODO: MOVE TO CPP FILE
		Signals()
		{
			// Initialize all handles to INVALID_HANDLE_VALUE to start with
			for(auto iterator : m_handles) iterator = INVALID_HANDLE_VALUE;

			SECURITY_ATTRIBUTES inherit = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
			m_handles[static_cast<size_t>(Signal::Ready)] = CreateEvent(&inherit, TRUE, FALSE, nullptr);
		}

		// Destructor
		//
		~Signals() { for(auto iterator : m_handles) if(iterator != INVALID_HANDLE_VALUE) CloseHandle(iterator); }

		// Array subscript operator
		//
		HANDLE operator[](Signal index) const { return m_handles[static_cast<size_t>(index)]; }

		// HANDLE* conversion operator
		//
		operator HANDLE*() { return m_handles.data(); }

		// Set

		// Reset

		// Wait
		// Wait (+ additional handles)


		// Count
		//
		// Number of handles in the array
		__declspec(property(get=getCount)) size_t Count;
		size_t getCount(void) const { return m_handles.size(); }

		// Handles
		//
		// Gets the array of handles
		__declspec(property(get=getHandles)) HANDLE* Handles;
		HANDLE* getHandles(void) { return m_handles.data(); }

	private:

		Signals(const Signals&)=delete;
		Signals& operator=(const Signals&)=delete;

		// Member Variables
		//
		std::array<HANDLE, 1>			m_handles;		// Contained array of HANDLEs
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	// CreateELF32 (static)
	//
	// Constructs a new process instance from an ELF32 binary file
	static std::unique_ptr<Process> CreateELF32(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle);

	// CreateELF64 (static)
	//
	// Constructs a new process instance from an ELF64 binary file
	static std::unique_ptr<Process> CreateELF64(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle);

	// CreateScriptInterpreter (static)
	//
	// Constructs a new process instance from an interpreter script
	static std::unique_ptr<Process> CreateScriptInterpreter(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle);

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<Host>				m_host;		// Hosted windows process
	//std::unique_ptr<AuxiliaryVector>	m_auxvec;	// Auxiliary vector
	//std::unique_ptr<Signals>			m_signals;	// Process signals
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESS_H_
