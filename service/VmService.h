//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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
#include <mutex>
#include <set>	// temp
#include <linux/elf.h>
#include "resource.h"
#include "CompressedStreamReader.h"
#include "CpioArchive.h"
#include "Exception.h"
#include "File.h"
#include "FileSystem.h"
#include "IndexPool.h"
#include "PathSplitter.h"
#include "Process.h"
#include "ProcessClass.h"
#include "RpcInterface.h"
#include "SystemLog.h"
#include "VirtualMachine.h"

// File systems
//
#include "HostFileSystem.h"
#include "ProcFileSystem.h"
#include "RootFileSystem.h"
#include "TempFileSystem.h"

#pragma warning(push, 4)

//---------------------------------------------------------------------------
// Class VmService
//
// TODO: oh so many words need to go here

// Needs to inherit from enable_shared_from_this<VmService> to trigger the 
// use of std::shared_ptr<> in servicelib.  Shared pointer currently needs to
// be used to implement VirtualMachine
//
// Should rename this to "Vm" or "VmInstance", VmService is accurate but is
// not really representative anymore since this is becoming the one-stop shop
// for the entire VirtualMachine interface implementation.  VmFileSystem still
// exists for now, but will likely be collapsed into this like the others

class VmService : public Service<VmService>, public VirtualMachine,	public std::enable_shared_from_this<VmService>
{
public:

	// TODO: Minimum PID is actually based on a constant multiplied by the number of CPUs (or something like that)
	VmService() : m_pidpool(MIN_PROCESS_INDEX) {}

	// VirtualMachine Implementation
	//
	virtual uapi::pid_t							AllocatePID(void);
	virtual void								ReleasePID(uapi::pid_t pid);
	virtual std::shared_ptr<Process>			CloneProcess(const std::shared_ptr<Process>& process, uint32_t flags, void* taskstate, size_t taskstatelen);
	virtual void								CloseProcess(const std::shared_ptr<Process>& process);
	virtual std::shared_ptr<Process>			FindProcessByHostID(uint32_t hostpid);

	// updated file system api
	virtual void								CheckPermissions(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, const uapi::char_t* path, int flags, uapi::mode_t mode);
	virtual void								CreateCharacterDevice(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, const uapi::char_t* path, uapi::mode_t mode, uapi::dev_t device);
	virtual void								CreateDirectory(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, const uapi::char_t* path, uapi::mode_t mode);
	virtual std::shared_ptr<FileSystem::Handle> CreateFile(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, const uapi::char_t* path, int flags, uapi::mode_t mode);
	virtual void								CreateSymbolicLink(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, const uapi::char_t* path, const uapi::char_t* target);
	virtual void								GetAbsolutePath(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& alias, uapi::char_t* path, size_t pathlen);
	virtual void								MountFileSystem(const uapi::char_t* source, const uapi::char_t* target, const uapi::char_t* filesystem, uint32_t flags, void* data, size_t datalen);
	virtual std::shared_ptr<FileSystem::Handle> OpenExecutable(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, const uapi::char_t* path);
	virtual std::shared_ptr<FileSystem::Handle> OpenFile(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, const uapi::char_t* path, int flags, uapi::mode_t mode);
	virtual size_t								ReadSymbolicLink(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, const uapi::char_t* path, uapi::char_t* buffer, size_t length);
	virtual std::shared_ptr<FileSystem::Alias>	ResolvePath(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, const uapi::char_t* path, int flags);

	virtual std::shared_ptr<FileSystem> getRootFileSystem(void) { return m_rootfs; }

	virtual const std::tstring&	GetProperty(VirtualMachine::Properties id);
	virtual size_t				GetProperty(VirtualMachine::Properties id, char_t* value, size_t length);
	virtual size_t				GetProperty(VirtualMachine::Properties id, wchar_t* value, size_t length);
	virtual void				SetProperty(VirtualMachine::Properties id, const std::tstring& value);
	virtual void				SetProperty(VirtualMachine::Properties id, const char_t* value);
	virtual void				SetProperty(VirtualMachine::Properties id, const char_t* value, size_t length);
	virtual void				SetProperty(VirtualMachine::Properties id, const wchar_t* value);
	virtual void				SetProperty(VirtualMachine::Properties id, const wchar_t* value, size_t length);

private:

	VmService(const VmService &rhs)=delete;
	VmService& operator=(const VmService &rhs)=delete;

	// TEST - HACK JOB TO SOLVE THE PROBLEM FOR NOW
	// GET RID OF THIS
	virtual std::shared_ptr<VirtualMachine> ToSharedPointer(void)
	{
		return shared_from_this();
	}

	// Service<> Control Handler Map
	//
	BEGIN_CONTROL_HANDLER_MAP(VmService)
		CONTROL_HANDLER_ENTRY(SERVICE_CONTROL_STOP, OnStop)
	END_CONTROL_HANDLER_MAP()

	// CreateProcess
	//
	// Creates a new hosted process instance from a file system binary
	std::shared_ptr<Process> CreateProcess(uapi::pid_t pid, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir, 
		const uapi::char_t* path, const uapi::char_t** arguments, const uapi::char_t** environment);

	// LoadInitialFileSystem
	//
	// Loads the initial file system from an initramfs CPIO archive
	void LoadInitialFileSystem(const std::shared_ptr<FileSystem::Alias>& target, const tchar_t* archivefile);

	FileSystemPtr MountProcFileSystem(const char_t* name, uint32_t flags, const void* data);

	// OnStart (Service)
	//
	// Invoked when the service is started
	void OnStart(int argc, LPTSTR* argv);

	// OnStop
	//
	// Invoked when the service is stopped
	void OnStop(void);

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// MIN_PROCESS_INDEX
	//
	// Minimum allowable process index (PID)
	static const int MIN_PROCESS_INDEX = 300;

	// fs_map_t
	//
	// Typedef for a map<> of available file systems, not concurrent
	using fs_map_t = std::map<std::string, FileSystem::mount_func>;

	// mount_map_t
	//
	// Typedef for a concurrent map<> of mounted file systems and the alias they are mounted in
	using mount_map_t = Concurrency::concurrent_unordered_map<FileSystem::AliasPtr, FileSystemPtr>;

	// process_map_t
	//
	// 
	using process_map_t = std::map<uapi::pid_t, std::shared_ptr<Process>>;

	// property_map_t
	//
	// Typedef for a concurrent map<> of property strings
	using property_map_t = Concurrency::concurrent_unordered_map<VirtualMachine::Properties, std::tstring>;

	//-------------------------------------------------------------------------
	// Member Variables

	property_map_t						m_properties;	// Collection of vm properties
	std::unique_ptr<SystemLog>			m_syslog;		// System Log
	std::shared_ptr<FileSystem>			m_procfs;		// PROCFS file system instance

	// PROCESSES
	//
	IndexPool<int32_t>					m_pidpool;		// Process/thread id pool
	std::shared_ptr<Process>			m_initprocess;	// initial process object

	// FILE SYSTEMS
	//
	std::mutex							m_fslock;		// File system critical section
	fs_map_t							m_availfs;		// Available file systems
	FileSystemPtr						m_rootfs;		// Root file system
	mount_map_t							m_mounts;		// Collection of mounted file systems

	process_map_t						m_processes;

	//
	// PARAMETERS PULLED BACK IN FROM VMSERVICEPARAMETERS CLASS
	// NEEDS CLEANUP
	//

	BEGIN_PARAMETER_MAP(VmService)
		// process
		PARAMETER_ENTRY(_T("process.host.32bit"),	process_host_32bit);	// String
		PARAMETER_ENTRY(_T("process.host.64bit"),	process_host_64bit);	// String
		PARAMETER_ENTRY(_T("process.host.timeout"), process_host_timeout);	// DWord
		PARAMETER_ENTRY(_T("systemlog.length"),		systemlog_length);		// DWord
		PARAMETER_ENTRY(_T("vm.initpath"),			vm_initpath);			// String
		PARAMETER_ENTRY(_T("vm.initramfs"),			vm_initramfs);			// String
	END_PARAMETER_MAP()

	// process
	StringParameter			process_host_32bit;
	StringParameter			process_host_64bit;
	DWordParameter			process_host_timeout { 10000 };

	// systemlog
	DWordParameter			systemlog_length { 512 KiB };

	// virtualmachine
	StringParameter			vm_initpath { _T("/sbin/init") };
	StringParameter			vm_initramfs;

};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMSERVICE_H_
