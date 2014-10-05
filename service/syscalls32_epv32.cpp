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
#include "Process.h"
#include "SystemCalls.h"
#include <syscalls32.h>

#pragma warning(push, 4)

#ifdef _M_X64
#error syscalls32_epv32 module is not intended for use in 64-bit builds; use syscalls32_epv64
#endif

// Context
//
// Represents the backing data for the sys32_context_t context handle for a client process
class Context
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Constructs a new Context instance
	static Context* Create(const RPC_CALL_ATTRIBUTES* attributes, SystemCalls* syscalls, const std::shared_ptr<Process>& process)
	{
		// Allocate the backing storage for the class object using MIDL_user_allocate
		void* instance = MIDL_user_allocate(sizeof(Context));
		if(!instance) return nullptr;

		// Use placement new to construct the object on the storage
		return new(instance) Context(attributes, syscalls, process);
	}

	// Destroy (static)
	//
	// Destroys and releases a Context instance
	static void Destroy(Context* context)
	{
		if(!context) return;

		// Manually invoke the object destructor and release the memory
		context->~Context();
		MIDL_user_free(context);
	}

	//-------------------------------------------------------------------------
	// Properties

	// ClientPID
	//
	// Gets the host process identifier
	__declspec(property(get=getClientPID)) DWORD ClientPID;
	DWORD getClientPID(void) const { return m_pid; }

	// Process
	//
	// Gets the process object instance
	__declspec(property(get=getProcess)) ProcessPtr Process;
	ProcessPtr getProcess(void) const { return m_process; }

	// SystemCalls
	//
	// Gets the contained SystemCalls instance pointer
	__declspec(property(get=getSystemCalls)) ::SystemCalls* SystemCalls;
	::SystemCalls* getSystemCalls(void) const { return m_syscalls; }

private:

	~Context()=default;
	Context(const Context&)=delete;
	Context& operator=(const Context&)=delete;

	// Instance Constructor
	//
	Context(const RPC_CALL_ATTRIBUTES* attributes, ::SystemCalls* syscalls, const ProcessPtr& process)
	{
		_ASSERTE(attributes);
		_ASSERTE(syscalls);

		m_pid = reinterpret_cast<DWORD>(attributes->ClientPID);
		m_syscalls = syscalls;
		m_process = process;
	}

	//-------------------------------------------------------------------------
	// Member Variables

	DWORD					m_pid;			// Client process identifier
	::SystemCalls*			m_syscalls;		// SystemCalls instance pointer
	ProcessPtr				m_process;		// Process object instance
};

//-----------------------------------------------------------------------------
// acquire_context
//
// Creates a new context handle for a client process attaching to the interface
//
// Arguments:
//
//	rpchandle		- RPC binding handle
//	startinfo		- [out] set to the startup information for the process
//	context			- [out] set to the newly allocated context handle

static HRESULT acquire_context(handle_t rpchandle, sys32_startup_info* startinfo, sys32_context_exclusive_t* context)
{
	uuid_t					objectid;			// RPC object identifier
	SystemCalls*			syscalls;			// SystemCalls interface
	RPC_CALL_ATTRIBUTES		attributes;			// Client attributes
	RPC_STATUS				rpcresult;			// Result from RPC function call

	// Acquire the object id for the interface connected to by the client
	rpcresult = RpcBindingInqObject(rpchandle, &objectid);
	if(rpcresult != RPC_S_OK) return HRESULT_FROM_WIN32(rpcresult);

	// Convert the object id into a SystemCalls instance
	syscalls = SystemCalls::FromObjectID(objectid);
	if(syscalls == nullptr) return E_NOINTERFACE;

	// Acquire the attributes of the calling process for the Context object
	memset(&attributes, 0, sizeof(RPC_CALL_ATTRIBUTES));
	attributes.Version = RPC_CALL_ATTRIBUTES_VERSION;
	attributes.Flags = RPC_QUERY_CLIENT_PID;

	rpcresult = RpcServerInqCallAttributes(rpchandle, &attributes);
	if(rpcresult != RPC_S_OK) return HRESULT_FROM_WIN32(rpcresult);

	// Look up the startup information for this process from the virtual machine
	std::shared_ptr<Process> process = syscalls->FindClientProcess((DWORD)attributes.ClientPID);

	// TODO: CHECK THESE POINTERS FOR OVERFLOW - THIS IS 32BIT INTERFACE
	startinfo->entry_point = reinterpret_cast<sys32_addr_t>(process->EntryPoint);
	startinfo->stack_image = reinterpret_cast<sys32_addr_t>(process->StackImage);
	startinfo->stack_image_length = static_cast<sys32_size_t>(process->StackImageLength);

	// Create a Context object instance to be converted into the context handle
	*context = reinterpret_cast<sys32_context_exclusive_t>(Context::Create(&attributes, syscalls, process));
	return (*context) ? S_OK : E_OUTOFMEMORY;
}

//-----------------------------------------------------------------------------
// release_context
//
// Releases a context handle proviously allocated with allocate_context
//
// Arguments:
//
//	context			- [in/out] contains the handle to release and will be set to null

static HRESULT release_context(sys32_context_exclusive_t* context)
{
	if((context == nullptr) || (*context == nullptr)) return E_POINTER;

	// Cast the context handle back into a Context instance and destroy it
	Context* instance = reinterpret_cast<Context*>(*context);
	Context::Destroy(instance);

	*context = nullptr;					// Reset context handle to null
	return S_OK;						// Context has been released
}

//---------------------------------------------------------------------------
// sys32_context_exclusive_t_rundown
//
// Invoked by the RPC runtime when a client has disconnected without properly
// releasing an allocated context handle
//
// Arguments:
//
//	context			- Context handle to be forcibly released

void __RPC_USER sys32_context_exclusive_t_rundown(sys32_context_exclusive_t context)
{
	// TODO: this may want to call sys_exit() instead to mimick a proper termination
	release_context(&context);
}

//-----------------------------------------------------------------------------
// syscalls32_epv32
//
// 32-bit system calls object for use with 32-bit builds

SystemCalls32_v1_0_epv_t syscalls32_epv32 = {

	/* sys32_acquire_context = */	acquire_context,
	/* sys32_release_context = */	release_context,

	// 122: sys32_uname
	[](sys32_context_t context, sys32_utsname* buf) -> sys32_long_t
	{
		Context* instance = reinterpret_cast<Context*>(context);

		// clean this up, don't cast new_utsname like this
		try { return instance->SystemCalls->newuname(instance->Process, (uapi::new_utsname*)(buf)); }
		catch(LinuxException& linuxex) { return -linuxex.Code; }
		catch(Exception&) { return -1; }		// TODO
		catch(...) { return -1; }				// TODO
	},

	// 006: sys32_close
	[](sys32_context_t context, sys32_int_t fd) -> sys32_long_t 
	{ 
		(context); (fd);
		//Context* instance = reinterpret_cast<Context*>(context);
		//return state->instance->blah(state->process, fd);
		return -1; 
	},
};

//---------------------------------------------------------------------------

#pragma warning(pop)
