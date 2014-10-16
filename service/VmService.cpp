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

std::shared_ptr<Process> VmService::FindClientProcess(uint32_t clientpid)
{
	// dummy for testing
	(clientpid);
	return m_initprocess;
}


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
	LARGE_INTEGER				qpcbias;			// QueryPerformanceCounter bias

	// The system log needs to know what value acts as zero for the timestamps,
	// therefore acquire this at the earliest possible opportunity
	QueryPerformanceCounter(&qpcbias);

	try {

		// Create the settings subsystem instance first
		m_settings = std::make_unique<VmSettings>(shared_from_this());

		//
		// SYSTEM LOG
		//

		// Create the system log instance and seed the time bias to now
		m_syslog = std::make_unique<VmSystemLog>(m_settings);
		m_syslog->TimestampBias = qpcbias.QuadPart;

		// TODO: Put a real log here with the zero-time bias and the size of the
		// configured system log
		m_syslog->Push("System log initialized");
		//// need the syslog overloads to do this better!

		//
		// VIRTUAL FILE SYSTEM
		//

		// clearly this is temporary code
		m_vfs = VmFileSystem::Create(RootFileSystem::Mount(nullptr));
		m_vfs->Mount(nullptr, "/", "tmpfs", 0, nullptr);

		// ??? PROCFS / SYSFS ???

		//
		// INITRAMFS
		//

		// Check that the initramfs archive file exists
		std::tstring initramfs = m_settings->InitialRamFileSystem;
		if(!File::Exists(initramfs.c_str())) throw Exception(E_INITRAMFSNOTFOUND, initramfs.c_str());

		// Attempt to extract the contents of the initramfs into the tempfs
		try { LoadInitialFileSystem(initramfs.c_str()); }
		catch(Exception& ex) { throw Exception(E_INITRAMFSEXTRACT, ex, initramfs.c_str(), ex.Message); }

		//
		// RPC INTERFACES
		//

		// TODO: I want to rework the RpcInterface thing at some point, but this
		// is a LOT cleaner than making the RPC calls here.  Should also have
		// some kind of rundown to deal with exceptions properly, need nested try/catches here

		// This may work better as a general single registrar in syscalls/SystemCalls
		// since the service really doesn't care that it has 2 RPC interfaces, they
		// both just come back to this single service instance via the entry point vectors
		syscall32_listener::Register(RPC_IF_AUTOLISTEN);
		syscall32_listener::AddObject(this->ObjectID32); 
		m_bindstr32 = syscall32_listener::GetBindingString(this->ObjectID32);
		// m_syslog->Push(something)
		OutputDebugString(L"BINDSTR32: ");
		OutputDebugString(m_bindstr32.c_str());
		OutputDebugString(L"\r\n");

#ifdef _M_X64
		// x64 builds also register the 64-bit system calls interface
		syscall64_listener::Register(RPC_IF_AUTOLISTEN);
		syscall64_listener::AddObject(this->ObjectID64);
		m_bindstr64 = syscall64_listener::GetBindingString(this->ObjectID64);
		// m_syslog->Push(something)
		OutputDebugString(L"BINDSTR64: ");
		OutputDebugString(m_bindstr64.c_str());
		OutputDebugString(L"\r\n");
#endif
	} 

	// Win32Exception and Exception can be translated into a ServiceException
	catch(Win32Exception& ex) { throw ServiceException(static_cast<DWORD>(ex.Code)); }
	catch(Exception& ex) { throw ServiceException(ex.HResult); }

	//
	// INITIALIZE PROCESS MANAGER
	//

	//
	// LAUNCH INIT
	//
	std::string initpath = std::to_string(m_settings->InitPath);
	const uapi::char_t* args[] = { "First Argument", "Second Argument", nullptr };
	m_initprocess = Process::Create(shared_from_this(), initpath.c_str(), args, nullptr);
	//proc->Terminate(0);
	m_initprocess->Resume();

	// seems to work to here, can use this to create the new in-proc ELF loader
	
	// need to convert h into a FileDescriptor? Perhaps process should accept Handle
	// instead, file descriptors are just per-process mappings of Handle
	
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
	syscall64_listener::RemoveObject(this->ObjectID64);
	syscall64_listener::Unregister(true);
#endif

	// Remove the 32-bit system calls RPC interface
	syscall32_listener::RemoveObject(this->ObjectID32);
	syscall32_listener::Unregister(true);

	// Shut down and destroy all of the virtual machine subsystems
	// (they hold shared_ptr<>s to this service class and will prevent
	// the destructor from being called -- clearly need to reevaluate this)

	m_vfs.reset();
	m_syslog.reset();
	m_settings.reset();
}

//
// API
//
// DO I WANT TO BREAK THESE OUT INTO SEPARATE .CPP FILES, ONE FOR EACH?  SOME WILL
// GET EXTRAORDINARILY COMPLICATED TO IMPLEMENT
//
__int3264 VmService::newuname(const ProcessPtr& process, uapi::new_utsname* buf)
{
	uapi::char_t	nodename[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD			cch = MAX_COMPUTERNAME_LENGTH + 1;

	UNREFERENCED_PARAMETER(process);

	if(buf == nullptr) throw LinuxException(LINUX_EFAULT);

	// Get the NetBIOS computer name to act as the node name, just null it out on erro
	if(!GetComputerNameA(nodename, &cch)) nodename[0] = '\0';

	// TODO: These are generally just placeholders, not even sure that "i686" is correct
	strncpy_s(buf->sysname, LINUX__NEW_UTS_LEN + 1, "TODO: Linux Emulator", _TRUNCATE);
	strncpy_s(buf->nodename, LINUX__NEW_UTS_LEN + 1, nodename, _TRUNCATE);
	strncpy_s(buf->release, LINUX__NEW_UTS_LEN + 1, "3.0.0-0-todo", _TRUNCATE);
	strncpy_s(buf->version, LINUX__NEW_UTS_LEN + 1, "TODO: Linux Emulator Version", _TRUNCATE);
#ifdef _M_X64
	strncpy_s(buf->machine, LINUX__NEW_UTS_LEN + 1, "x86_64", _TRUNCATE);
#else
	strncpy_s(buf->machine, LINUX__NEW_UTS_LEN + 1, "i686", _TRUNCATE);
#endif
	strncpy_s(buf->domainname, LINUX__NEW_UTS_LEN + 1, "TODO: DOMAINNAME", _TRUNCATE);

	return 0;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
