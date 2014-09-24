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

#include <memory>
#include <linux/elf.h>
#include "Exception.h"
#include "LinuxException.h"
#include "Host.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

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
	std::unique_ptr<Process> Create(const std::shared_ptr<VirtualMachine>& vm, const tchar_t* path);

private:

	Process(const Process&)=delete;
	Process& operator=(const Process&)=delete;

	// Instance Constructor
	//
	Process(std::unique_ptr<Host>&& host) : m_host(std::move(host)) {}
	friend std::unique_ptr<Process> std::make_unique<Process, std::unique_ptr<Host>>(std::unique_ptr<Host>&&);

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// ElfCommon_Ehdr
	//
	// Common portion of the ELF header structure, can be used to determine
	// what the size and type of the full header structure is
	typedef struct {

	  uint8_t		e_ident[LINUX_EI_NIDENT];
	  uint16_t		e_type;
	  uint16_t		e_machine;
	  uint32_t		e_version;

	} ElfCommon_Ehdr;

	//-------------------------------------------------------------------------
	// Member Variables

	std::unique_ptr<Host>		m_host;			// Host process instance
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESS_H_
