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

#ifndef __VMSERVICE_H_
#define __VMSERVICE_H_
#pragma once

#include <map>
#include "resource.h"
#include "SystemCalls.h"
#include "VmProcessManager.h"
#include "VmSystemLog.h"

#pragma warning(push, 4)			

//---------------------------------------------------------------------------
// Class VmService

class VmService : public Service<VmService>, private SystemCalls
{
public:

	// Instance Constructor
	//
	VmService();

	// CONTROL_HANDLER_MAP
	//
	BEGIN_CONTROL_HANDLER_MAP(VmService)
		CONTROL_HANDLER_ENTRY(SERVICE_CONTROL_STOP, OnStop)
	END_CONTROL_HANDLER_MAP()

	// PARAMETER_MAP
	//
	BEGIN_PARAMETER_MAP(VmService)
		PARAMETER_ENTRY(IDR_PARAM_INITRAMFS, m_initramfs)
		PARAMETER_ENTRY(IDR_PARAM_SYSLOGLENGTH, m_sysloglength)
	END_PARAMETER_MAP()

private:

	VmService(const VmService &rhs)=delete;
	VmService& operator=(const VmService &rhs)=delete;

	// OnStart (Service)
	//
	// Invoked when the service is started
	void OnStart(int argc, LPTSTR* argv);

	// OnStop
	//
	// Invoked when the service is stopped
	void OnStop(void);

	// m_initramfs
	//
	// Path to the virtual machine's initramfs blob
	StringParameter m_initramfs;


	std::unique_ptr<VmProcessManager> m_procmgr;

	// m_syslog
	//
	// System log implementation
	std::unique_ptr<VmSystemLog> m_syslog;
	DWordParameter m_sysloglength { 512 KiB };

	// m_vfs
	//
	// Virtual file system instance
	// VirtualFileSystem m_vfs;

	UUID m_objid32;
	std::tstring m_bindstr32;

#ifdef _M_X64
	UUID m_objid64;
	std::tstring m_bindstr64;
#endif

	//class Listener
	//{
	//public:

	//	explicit Listener(RPC_IF_HANDLE ifspec, const UUID& type, RPC_MGR_EPV* epv) : 
	//		m_interface(ifspec), m_type(type), m_epv(epv) {}

	//	~Listener() { Stop(); }

	//	RPC_STATUS Start(const UUID& object);
	//	RPC_STATUS Stop(void);

	//private:

	//	Listener(const Listener&)=delete;
	//	Listener& operator=(const Listener&)=delete;

	//	RPC_IF_HANDLE			m_interface = nullptr;
	//	UUID					m_type = GUID_NULL;
	//	RPC_MGR_EPV*			m_epv = nullptr;
	//	
	//	UUID					m_object = GUID_NULL;
	//	RPC_BINDING_VECTOR*		m_bindings = nullptr;
	//	std::tstring			m_bindstr;
	//};

	//Listener			m_listener32;
	//Listener			m_listener64;
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMSERVICE_H_
