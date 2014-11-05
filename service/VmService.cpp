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

// 32-bit listener
#include <syscalls32.h>
using syscall32_listener = RpcInterface<&SystemCalls32_v1_0_s_ifspec>;

#ifdef _M_X64
// 64-bit listener
#include <syscalls64.h>
using syscall64_listener = RpcInterface<&SystemCalls64_v1_0_s_ifspec>;
#endif

std::shared_ptr<Process> VmService::FindProcessByHostID(uint32_t hostpid)
{
	// dummy for testing
	(hostpid);
	return m_initprocess;
}

FileSystemPtr VmService::MountProcFileSystem(const char_t* name, uint32_t flags, const void* data)
{
	(name);
	(flags);
	(data);

	_ASSERTE(m_procfs);

	//
	// todo: can different mount options be used for different mounts of procfs?
	// typically there is only going to be one of these
	//
	// ANSWER: YES (need to deal with that - can use an std::bind() instance
	// of Mount() that references the underlying singleton?
	//

	// PROCFS is a singleton file system within a virtual machine instance, 
	// always return a reference to the existing instance
	return m_procfs;
}

//-----------------------------------------------------------------------------
// VmService::GetProperty
//
// Retrieves a property as an std::string instance
//
// Arguments:
//
//	id			- Property identifier

std::string VmService::GetProperty(VirtualMachine::Properties id)
{
	return std::string(m_properties[id]);
}

//-----------------------------------------------------------------------------
// VmService::GetProperty
//
// Retrieves a property into a character buffer
//
// Arguments:
//
//	id			- Property identifier
//	value		- Destination buffer to receive the value
//	length		- Length of the destination buffer

size_t VmService::GetProperty(VirtualMachine::Properties id, uapi::char_t* value, size_t length)
{
	std::string found = m_properties[id];

	strncpy_s(value, length, found.c_str(), _TRUNCATE);
	return strlen(value) + 1;
}

//-----------------------------------------------------------------------------
// VmService::SetProperty
//
// Sets the value of a property via an std::string instance
//
// Arguments:
//
//	id			- Property identifier
//	value		- Value to set/replace the existing value

void VmService::SetProperty(VirtualMachine::Properties id, std::string value)
{
	m_properties[id] = value;
}

//-----------------------------------------------------------------------------
// VmService::SetProperty
//
// Sets the value of a property via a null-terminated character buffer
//
// Arguments:
//
//	id			- Property identifier
//	value		- Value to set/replace the existing value

void VmService::SetProperty(VirtualMachine::Properties id, const uapi::char_t* value)
{
	m_properties[id] = std::move(std::string(value));
}

//-----------------------------------------------------------------------------
// VmService::SetProperty
//
// Sets the value of a property via an std::string instance
//
// Arguments:
//
//	id			- Property identifier
//	value		- Value to set/replace the existing value
//	length		- Length of the value character buffer, in bytes/chars

void VmService::SetProperty(VirtualMachine::Properties id, const uapi::char_t* value, size_t length)
{
	m_properties[id] = std::move(std::string(value, length));
}

// THIS SHOULD BE IN THE VFS (VMFILESYSTEM), NOT HERE IN THE SERVICE
void VmService::LoadInitialFileSystem(const tchar_t* archivefile)
{
	// Attempt to open the specified file read-only with sequential scan optimization
	std::unique_ptr<File> archive = File::OpenExisting(archivefile, GENERIC_READ, FILE_SHARE_READ, FILE_FLAG_SEQUENTIAL_SCAN);

	// Decompress as necessary and iterate over all the files contained in the CPIO archive
	CpioArchive::EnumerateFiles(CompressedStreamReader::FromFile(archive), [&](const CpioFile& file) -> void {

		// Convert the path string from ANSI/UTF8 to generic text for the FileSystem API
		std::string path = std::to_string(file.Path);

		// Depending on the type of node being enumerated, construct the appropriate object
		switch(file.Mode & LINUX_S_IFMT) {

			// TODO: These all need UID and GID support, likely will need to call chown() equivalent

			case LINUX_S_IFREG: 
			{
				FileSystem::HandlePtr p = m_vfs->CreateFile(path.c_str(), 0, file.Mode);
				std::vector<uint8_t> buffer(64 KiB);
				size_t read = file.Data.Read(buffer.data(), buffer.size());
				while(read > 0) {

					p->Write(buffer.data(), read);
					read = file.Data.Read(buffer.data(), buffer.size());
				}
			}
				break;

			case LINUX_S_IFDIR:
				m_vfs->CreateDirectory(path.c_str());
				break;

			case LINUX_S_IFLNK:
			{
				std::vector<char> buffer(file.Data.Length + 1);
				file.Data.Read(buffer.data(), buffer.size());
				std::string target = std::to_string(buffer.data());
				m_vfs->CreateSymbolicLinkW(path.c_str(), target.c_str());
			}
				break;
				
			case LINUX_S_IFCHR:
				//_RPTF0(_CRT_ASSERT, "initramfs: S_IFCHR not implemented yet");
				break;

			case LINUX_S_IFBLK:
				//_RPTF0(_CRT_ASSERT, "initramfs: S_IFBLK not implemented yet");
				break;

			case LINUX_S_IFIFO:
				//_RPTF0(_CRT_ASSERT, "initramfs: S_IFIFO not implemented yet");
				break;

			case LINUX_S_IFSOCK:
				//_RPTF0(_CRT_ASSERT, "initramfs: S_IFSOCK not implemented yet");
				break;

			default:
				_RPTF0(_CRT_ASSERT, "initramfs: unknown node type detected in archive");
				break;
		}
	});
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
	using namespace std::placeholders;

	LARGE_INTEGER				qpcbias;			// QueryPerformanceCounter bias

	// The system log needs to know what value acts as zero for the timestamps,
	// therefore acquire this at the earliest possible opportunity
	QueryPerformanceCounter(&qpcbias);

	try {

		//
		// PROPERTIES
		//
		SetProperty(Properties::DomainName, "DOMAIN NAME");
#ifndef _M_X64
		SetProperty(Properties::HardwareIdentifier, "i686");
#else
		SetProperty(Properties::HardwareIdentifier, "x86_64");
#endif
		SetProperty(Properties::HostName, "HOST NAME");
		SetProperty(Properties::OperatingSystemRelease, "3.0.0.0-todo");
		SetProperty(Properties::OperatingSystemType, "Linux");
		SetProperty(Properties::OperatingSystemVersion, "OS VERSION");

		//
		// SYSTEM LOG
		//

		// Create the system log instance and seed the time bias to now
		m_syslog = std::make_unique<VmSystemLog>(static_cast<uint32_t>(systemlog_length));
		m_syslog->TimestampBias = qpcbias.QuadPart;

		// TODO: Put a real log here with the zero-time bias and the size of the
		// configured system log
		m_syslog->Push("System log initialized");
		//// need the syslog overloads to do this better!

		//
		// VIRTUAL FILE SYSTEM
		//

		m_procfs = ProcFileSystem::Create();

		// clearly this is temporary code
		m_vfs = VmFileSystem::Create(RootFileSystem::Mount(nullptr, 0, nullptr));

		m_vfs->AddFileSystem("hostfs", HostFileSystem::Mount);
		m_vfs->AddFileSystem("procfs", std::bind(&VmService::MountProcFileSystem, this, _1, _2, _3));
		//m_vfs->AddFileSystem("rootfs", RootFileSystem::Mount);
		m_vfs->AddFileSystem("tmpfs", TempFileSystem::Mount);

		m_vfs->Mount(nullptr, "/", "tmpfs", 0, nullptr);
		m_vfs->CreateDirectory("/proc");
		m_vfs->Mount(nullptr, "/proc", "procfs", 0, nullptr);
		//m_vfs->Mount("D:\\Linux Stuff\\android-l-preview_r2-x86\\root", "/", "hostfs", 0, nullptr);

		// ??? PROCFS / SYSFS ???

		//
		// INITRAMFS
		//

		// Check that the initramfs archive file exists
		std::tstring initramfs = vm_initramfs;
		if(!File::Exists(initramfs.c_str())) throw Exception(E_INITRAMFSNOTFOUND, initramfs.c_str());

		// Attempt to extract the contents of the initramfs into the tempfs
		try { LoadInitialFileSystem(initramfs.c_str()); }
		catch(Exception& ex) { throw Exception(E_INITRAMFSEXTRACT, ex, initramfs.c_str(), ex.Message); }

		//
		// PROCESS MANAGER
		//
		m_procmgr = std::make_unique<VmProcessManager>();
		m_procmgr->HostPath32 = static_cast<std::tstring>(process_host_32bit).c_str();
#ifdef _M_X64
		m_procmgr->HostPath64 = static_cast<std::tstring>(process_host_64bit).c_str();
#endif

		//
		// RPC INTERFACES
		//

		// TODO: I want to rework the RpcInterface thing at some point, but this
		// is a LOT cleaner than making the RPC calls here.  Should also have
		// some kind of rundown to deal with exceptions properly, need nested try/catches here

		// This may work better as a general single registrar in syscalls/SystemCalls
		// since the service really doesn't care that it has 2 RPC interfaces, they
		// both just come back to this single service instance via the entry point vectors
		syscall32_listener::Register(RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY);
		syscall32_listener::AddObject(this->InstanceID);
		m_procmgr->HostArguments32 = syscall32_listener::GetBindingString(this->InstanceID).c_str();
		// m_syslog->Push(something)
		OutputDebugString(L"BINDSTR32: ");
		OutputDebugString(m_procmgr->HostArguments32);
		OutputDebugString(L"\r\n");

		// THESE ARE EXACTLY THE SAME, WHY DO I HAVE TWO OF THEM
		// collapse ProcessManager->HostArguments32 and 64 into just one property, the binding
		// string is identical, it's the interface that differs, which is controlled by the stubs

#ifdef _M_X64
		// x64 builds also register the 64-bit system calls interface
		syscall64_listener::Register(RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY);
		syscall64_listener::AddObject(this->InstanceID);
		m_procmgr->HostArguments64 = syscall64_listener::GetBindingString(this->InstanceID).c_str();
		// m_syslog->Push(something)
		OutputDebugString(L"BINDSTR64: ");
		OutputDebugString(m_procmgr->HostArguments64);
		OutputDebugString(L"\r\n");
#endif
	} 

	// Win32Exception and Exception can be translated into a ServiceException
	catch(Win32Exception& ex) { throw ServiceException(static_cast<DWORD>(ex.Code)); }
	catch(Exception& ex) { throw ServiceException(ex.HResult); }

	//
	// LAUNCH INIT
	//
	std::string initpath = std::to_string(vm_initpath);
	const uapi::char_t* args[] = { "First Argument", "Second Argument", nullptr };
	//why is shared_from_this() null here
	//auto test = VirtualMachine::shared_from_this();
	m_initprocess = m_procmgr->CreateProcess(shared_from_this(), initpath.c_str(), args, nullptr);
	////proc->Terminate(0);
	m_initprocess->Resume();

	// seems to work to here, can use this to create the new in-proc ELF loader
	
	// need to maintain a reference to the Process object since
	// this will need a watcher thread to panic if init stops before service stops
	// other processes will be caught by RPC context rundown

	// TODO: throwing an exception from here messes up the ServiceHarness, you get
	// that damn "abort() has been called" from the std::thread ... fix that 
	// in servicelib at some point in the near future
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

#ifdef _M_X64
	// Remove the 64-bit system calls RPC interface
	syscall64_listener::RemoveObject(this->InstanceID);
	syscall64_listener::Unregister(true);
#endif

	// Remove the 32-bit system calls RPC interface
	syscall32_listener::RemoveObject(this->InstanceID);
	syscall32_listener::Unregister(true);

	// Shut down and destroy all of the virtual machine subsystems
	// (they hold shared_ptr<>s to this service class and will prevent
	// the destructor from being called -- clearly need to reevaluate this)

	//m_procmgr.reset();
	//m_propmgr.reset();
	//m_vfs.reset();
	//m_syslog.reset();
}

FileSystem::HandlePtr VmService::OpenExecutable(const uapi::char_t* path)
{
	_ASSERTE(m_vfs);
	return m_vfs->OpenExec(path);
}

FileSystem::HandlePtr VmService::OpenFile(const uapi::char_t* pathname, int flags, uapi::mode_t mode)
{
	_ASSERTE(m_vfs);
	return m_vfs->Open(pathname, flags, mode);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
