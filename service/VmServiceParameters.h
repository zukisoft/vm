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

#ifndef __VMSERVICEPARAMETERS_H_
#define __VMSERVICEPARAMETERS_H_
#pragma once

#include <functional>
#include <servicelib.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// VmServiceParameters
//
// Implements the servicelib parameters externally to the VmService class so 
// that they can be shared with VmSettings
//
// TODO: This is a hack for now, build support for this directly into servicelib
// to allow a service to access an external parameters class instance

class VmServiceParameters
{
public:

	// Constructor / Destructor
	//
	VmServiceParameters()=default;
	virtual ~VmServiceParameters()=default;

	//-------------------------------------------------------------------------
	// Fields

	// BinaryParameter		-> svctl::parameter<_type, ServiceParameterFormat::Binary>
	// DWordParameter		-> svctl::parameter<uint32_t, ServiceParameterFormat::DWord>
	// MultiStringParameter	-> svctl::parameter<std::vector<svctl::tstring>, ServiceParameterFormat::MultiString, svctl::tstring>
	// QWordParameter		-> svctl::parameter<uint64_t, ServiceParameterFormat::QWord>
	// StringParameter		-> svctl::parameter<svctl::tstring, ServiceParameterFormat::String>

	// process
	svctl::parameter<svctl::tstring, ServiceParameterFormat::String>	process_host_32bit;
	svctl::parameter<svctl::tstring, ServiceParameterFormat::String>	process_host_64bit;
	svctl::parameter<uint32_t, ServiceParameterFormat::DWord>			process_host_timeout { 10000 };

	// systemlog
	svctl::parameter<uint32_t, ServiceParameterFormat::DWord>			systemlog_length { 512 KiB };

	// virtualmachine
	svctl::parameter<svctl::tstring, ServiceParameterFormat::String>	vm_initpath { _T("/sbin/init") };
	svctl::parameter<svctl::tstring, ServiceParameterFormat::String>	vm_initramfs;

protected:

	// BEGIN_PARAMETER_MAP Equivalent
	//
	void IterateParameters(std::function<void(const svctl::tstring& name, svctl::parameter_base& param)> func)
	{
		// process
		PARAMETER_ENTRY(_T("process.host.32bit"), process_host_32bit);			// String
		PARAMETER_ENTRY(_T("process.host.64bit"), process_host_64bit);			// String
		PARAMETER_ENTRY(_T("process.host.timeout"), process_host_timeout);		// DWord

		// systemlog
		PARAMETER_ENTRY(_T("systemlog.length"), systemlog_length);				// DWord

		// virtualmachine
		PARAMETER_ENTRY(_T("vm.initpath"), vm_initpath);						// String
		PARAMETER_ENTRY(_T("vm.initramfs"), vm_initramfs);						// String
	}

private:

	VmServiceParameters(const VmServiceParameters&)=delete;
	VmServiceParameters& operator=(const VmServiceParameters&)=delete;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMSERVICEPARAMETERS_H_
