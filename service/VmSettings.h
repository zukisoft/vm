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

#ifndef __VMSETTINGS_H_
#define __VMSETTINGS_H_
#pragma once

#include "VmServiceParameters.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// VmSettings
//
// Virtual Machine Settings subsystem, collects and exposes all VM settings
// in a hierarchical view based on the subsystem to which they apply

class VmSettings
{
private:

	class ProcessSettings;
	class SystemLogSettings;

public:

	// Constructor / Destructor
	//
	VmSettings(VmServiceParameters* parameters) : m_parameters(parameters), m_process(parameters), m_systemlog(parameters) {}
	~VmSettings()=default;

	//-------------------------------------------------------------------------
	// Properties

	// InitPath
	//
	// Gets the path to the init binary for the virtual machine instance
	__declspec(property(get=getInitPath)) svctl::tstring InitPath;
	svctl::tstring getInitPath(void) { return m_parameters->vm_initpath; }

	// InitialRamFileSystem
	//
	// Gets the path to the initramfs archive for the virtual machine instance
	__declspec(property(get=getInitialRamFileSystem)) svctl::tstring InitialRamFileSystem;
	svctl::tstring getInitialRamFileSystem(void) { return m_parameters->vm_initramfs; }

	// Process
	//
	// Gets a reference to the contained ProcessSettings instance
	__declspec(property(get=getProcess)) ProcessSettings& Process;
	ProcessSettings& getProcess(void) { return m_process; }

	// SystemLog
	//
	// Gets a reference to the contained SystemLogSettings instance
	__declspec(property(get=getSystemLog)) SystemLogSettings& SystemLog;
	SystemLogSettings& getSystemLog(void) { return m_systemlog; }

private:

	VmSettings(const VmSettings&)=delete;
	VmSettings& operator=(const VmSettings&)=delete;

	// ProcessSettings
	//
	// Settings specific to the process manager subsystem
	class ProcessSettings
	{
	public:

		ProcessSettings(VmServiceParameters* parameters) : m_parameters(parameters) {}

		// Host32
		//
		// Gets the path to the 32-bit hosting executable
		__declspec(property(get=getHost32)) svctl::tstring Host32;
		svctl::tstring getHost32(void) { return m_parameters->process_host_32bit; }
	
		// Host64
		//
		// Gets the path to the 64-bit hosting executable
		__declspec(property(get=getHost64)) svctl::tstring Host64;
		svctl::tstring getHost64(void) { return m_parameters->process_host_64bit; }

		// HostTimeout
		//
		// Gets the hosting executable callback timeout
		__declspec(property(get=getHostTimeout)) uint32_t HostTimeout;
		uint32_t getHostTimeout(void) { return m_parameters->process_host_timeout; }
	
	private:

		VmServiceParameters* m_parameters;
	};

	// SystemLogSettings
	//
	// Settings specific to the system log subsystem
	class SystemLogSettings
	{
	public:

		SystemLogSettings(VmServiceParameters* parameters) : m_parameters(parameters) {}

		// Length
		//
		// todo
		__declspec(property(get=getLength)) uint32_t Length;
		uint32_t getLength(void) { return m_parameters->systemlog_length; }
	
	private:

		VmServiceParameters* m_parameters;
	};

	//-------------------------------------------------------------------------
	// Member Variables

	VmServiceParameters*	m_parameters;
	
	ProcessSettings							m_process;
	SystemLogSettings						m_systemlog;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMSETTINGS_H_
