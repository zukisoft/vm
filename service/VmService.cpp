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

// INTERPRETER_SCRIPT_MAGIC
//
// Magic number present at the head of an interpreter script
static uint8_t INTERPRETER_SCRIPT_MAGIC[] = { 0x23, 0x21 };		// "#!"

void VmService::CheckPermissions(const uapi::char_t* path, uapi::mode_t mode)
{
	_ASSERTE(m_vfs);

	// Locate the node within the virtual file system, throw ENOENT if not found
	auto node = m_vfs->ResolvePath(path)->Node;
	if(!node) throw LinuxException(LINUX_ENOENT);

	// Node was located, demand the requested permissions from it
	node->DemandPermission(mode);
}

//-----------------------------------------------------------------------------
// VmService::CreateProcess (private)
//
// Creates a new Process instance from a file system binary
//
// Arguments:
//
//	rootdir			- Initial root directory for the new process
//	workingdir		- Initial working directory for the new process
//	path			- Path to the file system object to execute as a process
//	arguments		- Pointer to an array of command line argument strings
//	environment		- Pointer to the process environment variables

std::shared_ptr<Process> VmService::CreateProcess(const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir, 
	const uapi::char_t* path, const uapi::char_t** arguments, const uapi::char_t** environment)
{
	if(!path) throw LinuxException(LINUX_EFAULT);

	// Attempt to open an execute handle for the specified path
	FileSystem::HandlePtr handle = OpenExecutable(path);

	// Read in enough data from the head of the file to determine the type
	uint8_t magic[LINUX_EI_NIDENT];
	size_t read = handle->Read(magic, LINUX_EI_NIDENT);

	// ELF BINARY
	//
	if((read >= LINUX_EI_NIDENT) && (memcmp(magic, LINUX_ELFMAG, LINUX_SELFMAG) == 0)) {

		switch(magic[LINUX_EI_CLASS]) {

			// ELFCLASS32: Create a 32-bit host process for the binary
			// TODO: clean up the arguments, I hate c_str(). need to work on svctl::parameter
			case LINUX_ELFCLASS32: 
				return Process::Create<ElfClass::x86>(shared_from_this(), rootdir, workingdir, handle, arguments, environment, 
					((svctl::tstring)process_host_32bit).c_str(), m_hostarguments32.c_str());
#ifdef _M_X64
			// ELFCLASS64: Create a 64-bit host process for the binary
			case LINUX_ELFCLASS64: 
				return Process::Create<ElfClass::x86_64>(shared_from_this(), rootdir, workingdir, handle, arguments, environment, 
					((svctl::tstring)process_host_64bit).c_str(), m_hostarguments64.c_str());
#endif
			// Any other ELFCLASS -> ENOEXEC	
			default: throw LinuxException(LINUX_ENOEXEC);
		}
	}

	// INTERPRETER SCRIPT
	//
	else if((read >= sizeof(INTERPRETER_SCRIPT_MAGIC)) && (memcmp(magic, &INTERPRETER_SCRIPT_MAGIC, sizeof(INTERPRETER_SCRIPT_MAGIC)) == 0)) {

		char_t *begin, *end;					// String tokenizing pointers

		// Move the file pointer back to the position immediately after the magic number
		handle->Seek(sizeof(INTERPRETER_SCRIPT_MAGIC), LINUX_SEEK_SET);

		// Read up to the allocated buffer's worth of data from the file
		HeapBuffer<uapi::char_t> buffer(MAX_PATH);
		char_t *eof = &buffer + handle->Read(&buffer, buffer.Size);

		// Find the interperter string, if not present the script is invalid
		for(begin = &buffer; (begin < eof) && (*begin) && (*begin != '\n') && (isspace(*begin)); begin++);
		for(end = begin; (end < eof) && (*end) && (*end != '\n') && (!isspace(*end)); end++);
		if(begin == end) throw LinuxException(LINUX_ENOEXEC);
		std::string interpreter(begin, end);

		// Find the optional argument string
		for(begin = end; (begin < eof) && (*begin) && (*begin != '\n') && (isspace(*begin)); begin++);
		for(end = begin; (end < eof) && (*end) && (*end != '\n') && (!isspace(*end)); end++);
		std::string argument(begin, end);

		// Create a new argument array to pass back in, using the parsed interpreter and argument
		std::vector<const char_t*> newarguments;
		newarguments.push_back(interpreter.c_str());
		if(argument.length()) newarguments.push_back(argument.c_str());
		newarguments.push_back(path);

		// Append the original argv[1] .. argv[n] pointers to the new argument array
		if(arguments && (*arguments)) arguments++;
		while((arguments) && (*arguments)) { newarguments.push_back(*arguments); arguments++; }
		newarguments.push_back(nullptr);

		// Recursively call back into CreateProcess with the interpreter path and arguments
		return CreateProcess(rootdir, workingdir, interpreter.c_str(), newarguments.data(), environment);
	}

	// UNSUPPORTED BINARY FORMAT
	//
	throw LinuxException(LINUX_ENOEXEC);
}

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
		m_syslog = std::make_unique<SystemLog>(static_cast<uint32_t>(systemlog_length));
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

		//m_vfs->Mount(nullptr, "/", "tmpfs", 0, nullptr);
		m_vfs->Mount("D:\\Linux Stuff\\android-l-preview_r2-x86\\root", "/", "hostfs", 0, nullptr);

		// ??? PROCFS / SYSFS ???
		//m_vfs->CreateDirectory("/proc");
		//m_vfs->Mount(nullptr, "/proc", "procfs", 0, nullptr);


		//
		// INITRAMFS
		//

		// Use the existence of the setting to determine if initramfs feature should be used
		std::tstring initramfs = vm_initramfs;
		if(initramfs.length()) {

			// Ensure that the initramfs file actually exists on the host file system
			if(!File::Exists(initramfs.c_str())) throw Exception(E_INITRAMFSNOTFOUND, initramfs.c_str());

			// Attempt to extract the contents of the initramfs into the current root file system
			try { LoadInitialFileSystem(initramfs.c_str()); }
			catch(Exception& ex) { throw Exception(E_INITRAMFSEXTRACT, ex, initramfs.c_str(), ex.Message); }
		}

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
		m_hostarguments32 = syscall32_listener::GetBindingString(this->InstanceID).c_str();
		// m_syslog->Push(something)
		OutputDebugString(L"BINDSTR32: ");
		OutputDebugString(m_hostarguments32.c_str());
		OutputDebugString(L"\r\n");

		// THE BINDING STRINGS ARE EXACTLY THE SAME, WHY DO I HAVE TWO OF THEM
		// collapse ProcessManager->HostArguments32 and 64 into just one property, the binding
		// string is identical, it's the interface that differs, which is controlled by the stubs

#ifdef _M_X64
		// x64 builds also register the 64-bit system calls interface
		syscall64_listener::Register(RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY);
		syscall64_listener::AddObject(this->InstanceID);
		m_hostarguments64 = syscall64_listener::GetBindingString(this->InstanceID).c_str();
		// m_syslog->Push(something)
		OutputDebugString(L"BINDSTR64: ");
		OutputDebugString(m_hostarguments64.c_str());
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
	const uapi::char_t* args[] = { initpath.c_str(), "First Argument", "Second Argument", nullptr };
	// TODO: NEED INITIAL ENVIRONMENT
	m_initprocess = CreateProcess(m_vfs->Root, m_vfs->Root, initpath.c_str(), args, nullptr);
	m_initprocess->Resume();

	// TODO: EXCEPTION HANDLING FOR INIT PROCESS -- DIFFERENT?
	// TODO: MONITOR INIT PROCESS

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

FileSystem::HandlePtr VmService::OpenFile(const FileSystem::AliasPtr& base, const uapi::char_t* pathname, int flags, uapi::mode_t mode)
{
	_ASSERTE(m_vfs);
	return m_vfs->Open(base, pathname, flags, mode);
}

size_t VmService::ReadSymbolicLink(const uapi::char_t* path, uapi::char_t* buffer, size_t length)
{
	_ASSERTE(m_vfs);

	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);
	if(length == 0) throw LinuxException(LINUX_ENOENT);

	auto node = std::dynamic_pointer_cast<FileSystem::SymbolicLink>(m_vfs->ResolvePath(path)->Node);
	if(!node) throw LinuxException(LINUX_ENOENT);

	return node->ReadTarget(buffer, length);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
