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

#include "stdafx.h"
#include "VmService.h"

#pragma warning(push, 4)

//---------------------------------------------------------------------------
// VmService Constructor
//
// Arguments:
//
//	NONE

VmService::VmService()
{
	// Construct unique identifiers for the system call interfaces by
	// embedding the service instance pointer into the first 32/64 bits of
	// the predefined UUIDs that represent the EPV types

	m_objid32 = UUID_SYSTEMCALLS32;
	*reinterpret_cast<VmService**>(&m_objid32) = this;

#ifdef _M_X64
	m_objid64 = UUID_SYSTEMCALLS64;
	*reinterpret_cast<VmService**>(&m_objid64) = this;
#endif
}

//---------------------------------------------------------------------------
// VmService::OnStart (private)
//
// Invoked when the service is started
//
// Arguments :
//
//	argc		- Number of command line arguments
//	argv		- Array of command line argument strings

void VmService::OnStart(int, LPTSTR*)
{
	RPC_STATUS					rpcresult;			// Result from function call
	LARGE_INTEGER				qpcbias;			// QueryPerformanceCounter bias

	// The system log needs to know what value acts as zero for the timestamps
	QueryPerformanceCounter(&qpcbias);

	// Create the system log instance and seed the time bias to now
	m_syslog = std::make_unique<VmSystemLog>(m_sysloglength);
	m_syslog->TimestampBias = qpcbias.QuadPart;

	// TODO: Put a real log here with the zero-time bias and the size of the
	// configured system log
	m_syslog->Push("System log initialized");
	////

	svctl::tstring initramfs = m_initramfs;

	// Attept to register the remote system call RPC interface
	RpcObjectSetType(&m_objid32, &UUID_SYSTEMCALLS32);
	rpcresult = RpcServerRegisterIfEx(SystemCalls32_v1_0_s_ifspec, &UUID_SYSTEMCALLS32, &syscalls32_32, RPC_IF_AUTOLISTEN, RPC_C_LISTEN_MAX_CALLS_DEFAULT, nullptr);
	if(rpcresult != RPC_S_OK) throw std::exception("RpcServerRegisterIf");

	RPC_BINDING_VECTOR* vector;
	rpcresult = RpcServerInqBindings(&vector);
	if(rpcresult == RPC_S_OK) {

		wchar_t* t;
		RPC_BINDING_HANDLE copy;
		RpcBindingCopy(vector->BindingH[0], &copy);
		RpcBindingSetObject(copy, &m_objid32);

		RpcBindingToStringBinding(copy, (RPC_WSTR*)&t);
		m_bindstr32 = t;
		RpcStringFree((RPC_WSTR*)&t);
		OutputDebugString(m_bindstr32.c_str());
		OutputDebugString(L"\r\n");

		UUID_VECTOR u = { 1, &m_objid32 };
		rpcresult = RpcEpRegister(SystemCalls32_v1_0_s_ifspec, vector, &u, nullptr);
		if(rpcresult == RPC_S_OK) {

			int x = 123;
		}
	}
	RpcBindingVectorFree(&vector);

	// Attempt to load the initial ramdisk file system
	// this needs unwind on exception?
	// m_vfs.LoadInitialFileSystem(initramfs.c_str());
}

//-----------------------------------------------------------------------------
// VmService::OnStop (private)
//
// Invoked when the service is stopped
//
// Arguments:
//
//	NONE

void VmService::OnStop(void)
{
	RPC_BINDING_VECTOR* vector;
	RPC_STATUS rpcresult = RpcServerInqBindings(&vector);

	UUID_VECTOR u = { 1, &m_objid32 };
	rpcresult = RpcEpUnregister(SystemCalls32_v1_0_s_ifspec, vector, &u);
	RpcBindingVectorFree(&vector);

	RpcServerUnregisterIf(SystemCalls32_v1_0_s_ifspec, &UUID_SYSTEMCALLS32, 1);

	UUID nil;
	UuidCreateNil(&nil);
	RpcObjectSetType(&m_objid32, &nil);
}

//-----------------------------------------------------------------------------

//
//RPC_STATUS VmService::Listener::Start(const UUID& object)
//{
//	RPC_STATUS			rpcresult;				// Result from RPC function call
//
//	m_object = object;
//
//	// Set the object->type->epv mapping for this instance
//	RpcObjectSetType(&m_object, &m_type);
//
//	// Attempt to register the provided interface with the provided type
//	rpcresult = RpcServerRegisterIfEx(m_interface, &m_type, m_epv, RPC_IF_AUTOLISTEN, RPC_C_LISTEN_MAX_CALLS_DEFAULT, nullptr);
//	if(rpcresult != RPC_S_OK) {
//		
//		RpcObjectSetType(&m_object, nullptr);
//		throw ServiceException(rpcresult);
//	}
//
//	// Retrieve the binding vector for this process
//	rpcresult = RpcServerInqBindings(&m_bindings);
//	if(rpcresult != RPC_S_OK) {
//
//		RpcServerUnregisterIf(m_interface, &m_type, TRUE);
//		RpcObjectSetType(&m_object, nullptr);
//		throw ServiceException(rpcresult);
//	}
//
//	// Register the endpoint for clients to connect to this object
//	UUID_VECTOR uuids = { 1, &m_object };
//	rpcresult = RpcEpRegister(m_interface, m_bindings, &uuids, nullptr);
//	if(rpcresult != RPC_S_OK) {
//
//		RpcBindingVectorFree(&m_bindings);
//		RpcServerUnregisterIf(m_interface, &m_type, TRUE);
//		RpcObjectSetType(&m_object, nullptr);
//		throw ServiceException(rpcresult);
//	}
//
//	// Finally, generate a binding string that describes this listener
//	wchar_t* bindstr;
//	RPC_BINDING_HANDLE copy;
//	RpcBindingCopy(m_bindings->BindingH[0], &copy);
//	RpcBindingSetObject(copy, &m_object);
//	RpcBindingToStringBinding(copy, reinterpret_cast<RPC_WSTR*>(&bindstr));
//	m_bindstr = bindstr;
//	RpcStringFree(reinterpret_cast<RPC_WSTR*>(&bindstr));
//
//	return RPC_S_OK;
//}
//
//RPC_STATUS VmService::Listener::Stop(void)
//{
//	RPC_STATUS			rpcresult;		// Result from RPC function call
//
//	m_bindstr = _T("");					// Remove the binding string
//
//	// Remove the endpoint that was generated for this interface
//	UUID_VECTOR uuids = { 1, &m_object };
//	RpcEpUnregister(m_interface, m_bindings, &uuids);
//
//	// Release and reset the binding vector
//	RpcBindingVectorFree(&m_bindings);
//	m_bindings = nullptr;
//
//	// Unregister the interface and remove the object UUID
//	RpcServerUnregisterIf(m_interface, &m_type, TRUE);
//	RpcObjectSetType(&m_object, nullptr);
//	m_object = GUID_NULL;
//
//	return RPC_S_OK;
//}

//---------------------------------------------------------------------------

#pragma warning(pop)
