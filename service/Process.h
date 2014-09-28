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
	~Process()=default;

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
	void Resume(void) { _ASSERTE(m_host); m_host->Resume(); }

	// Suspend
	//
	// Suspends the process
	void Suspend(void) { _ASSERTE(m_host); m_host->Suspend(); }

	// Terminate
	//
	// Terminates the process
	void Terminate(int exitcode) { _ASSERTE(m_host); m_host->Terminate(-exitcode); }

private:

	Process(const Process&)=delete;
	Process& operator=(const Process&)=delete;

	// Instance Constructor
	//
	Process(std::unique_ptr<Host>&& host) : m_host(std::move(host)) {}
	friend std::unique_ptr<Process> std::make_unique<Process, std::unique_ptr<Host>>(std::unique_ptr<Host>&&);

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// MagicNumbers
	//
	// Union that defines the magic numbers for supported binary formats
	typedef union {

		uint8_t	ANSI[3];				// 0x23, 0x21, 0x20
		uint8_t	UTF8[6];				// 0xEF, 0xBB, 0xBF, 0x23, 0x21, 0x20
		uint8_t	UTF16[8];				// 0xFF, 0xFE, 0x23, 0x00, 0x21, 0x00, 0x20, 0x00
		uint8_t	ELF[LINUX_EI_NIDENT];	// "\177ELF"
	
	} MagicNumbers;

	//-------------------------------------------------------------------------
	// Private Member Functions

	// Create (static)
	//
	// Creates a new process instance via an external Windows host binary
	template <int elfclass>
	static std::unique_ptr<Process> Create(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle,
		const uapi::char_t** argv, const uapi::char_t** envp, const tchar_t* hostpath, const tchar_t* hostargs);

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<Host>		m_host;		// Hosted windows process
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESS_H_
