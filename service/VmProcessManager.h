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

#ifndef __VMPROCESSMANAGER_H_
#define __VMPROCESSMANAGER_H_
#pragma once

#include <linux/types.h>
#include "Exception.h"
#include "LinuxException.h"
#include "Process.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// VmProcessManager
//
// Manages all processes hosted by a virtual machine instance

class VmProcessManager
{
public:

	VmProcessManager()=default;
	~VmProcessManager()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// CreateProcess
	//
	// Creates a new hosted process instance from a file system binary
	std::shared_ptr<Process> CreateProcess(const std::shared_ptr<VirtualMachine>& vm, const uapi::char_t* path, 
		const uapi::char_t** arguments, const uapi::char_t** environment);

	//-------------------------------------------------------------------------
	// Properties

	// HostArguments32
	//
	// Gets/Sets the command line arguments to be passed to the 32-bit host process
	__declspec(property(get=getHostArguments32, put=putHostArguments32)) const tchar_t* HostArguments32;
	const tchar_t* getHostArguments32(void) const { return m_hostargs32.c_str(); }
	void putHostArguments32(const tchar_t* value) { m_hostargs32 = value; }

	// HostPath32
	//
	// Gets/Sets the path to the 32-bit hosting process
	__declspec(property(get=getHostPath32, put=putHostPath32)) const tchar_t* HostPath32;
	const tchar_t* getHostPath32(void) const { return m_hostpath32.c_str(); }
	void putHostPath32(const tchar_t* value) { m_hostpath32 = value; }

#ifdef _M_X64
	// HostArguments64
	//
	// Gets/Sets the command line arguments to be passed to the 64-bit host process
	__declspec(property(get=getHostArguments64, put=putHostArguments64)) const tchar_t* HostArguments64;
	const tchar_t* getHostArguments64(void) const { return m_hostargs64.c_str(); }
	void putHostArguments64(const tchar_t* value) { m_hostargs64 = value; }

	// HostPath64
	//
	// Gets/Sets the path to the 64-bit hosting process
	__declspec(property(get=getHostPath64, put=putHostPath64)) const tchar_t* HostPath64;
	const tchar_t* getHostPath64(void) const { return m_hostpath64.c_str(); }
	void putHostPath64(const tchar_t* value) { m_hostpath64 = value; }
#endif

private:

	VmProcessManager(const VmProcessManager&)=delete;
	VmProcessManager& operator=(const VmProcessManager&)=delete;

	//-------------------------------------------------------------------------
	// Private Member Functions

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// MagicNumbers
	//
	// Union that defines the magic numbers for supported binary formats
	typedef union {

		uint8_t	AnsiScript[3];				// 0x23, 0x21, 0x20
		uint8_t	UTF8Script[6];				// 0xEF, 0xBB, 0xBF, 0x23, 0x21, 0x20
		uint8_t	UTF16Script[8];				// 0xFF, 0xFE, 0x23, 0x00, 0x21, 0x00, 0x20, 0x00
		uint8_t	ElfBinary[LINUX_EI_NIDENT];	// "\177ELF"
	
	} MagicNumbers;

	//-------------------------------------------------------------------------
	// Member Variables

	std::tstring			m_hostpath32;			// 32-bit host process path
	std::tstring			m_hostargs32;			// 32-bit host arguments

#ifdef _M_X64
	std::tstring			m_hostpath64;			// 64-bit host process path
	std::tstring			m_hostargs64;			// 64-bit host arguments
#endif
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMPROCESSMANAGER_H_
