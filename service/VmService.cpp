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

void VmService::LoadInitialFileSystem(const tchar_t* archivefile)
{
	// Attempt to open the specified file read-only with sequential scan optimization
	std::unique_ptr<File> archive = File::OpenExisting(archivefile, GENERIC_READ, FILE_SHARE_READ, FILE_FLAG_SEQUENTIAL_SCAN);

	// Decompress as necessary and iterate over all the files contained in the CPIO archive
	CpioArchive::EnumerateFiles(CompressedStreamReader::FromFile(archive), [&](const CpioFile& file) -> void {

		// Convert the path string from ANSI/UTF8 to generic text for the FileSystem API
		std::tstring path = std::to_tstring(file.Path);

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
				std::tstring target = std::to_tstring(buffer.data());
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
		m_settings = std::make_unique<VmSettings>(this);

		// Create the system log instance and seed the timestamp bias to now
		m_syslog = std::make_unique<VmSystemLog>(m_settings);
		m_syslog->TimestampBias = qpcbias.QuadPart;

		//
		// VIRTUAL FILE SYSTEM
		//

		// clearly this is temporary code
		m_vfs = VmFileSystem::Create(RootFileSystem::Mount(nullptr));
		m_vfs->Mount(nullptr, L"/", L"tmpfs", 0, nullptr);

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

#ifdef _M_X64
		// x64 builds also register the 64-bit system calls interface
		syscall64_listener::Register(RPC_IF_AUTOLISTEN);
		syscall64_listener::AddObject(this->ObjectID64);
		m_bindstr64 = syscall64_listener::GetBindingString(this->ObjectID64);
		// m_syslog->Push(something)
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
	std::unique_ptr<Process> p = Process::Create(this);

	std::tstring initpath = m_settings->InitPath;
	FileSystem::HandlePtr h = m_vfs->Open(initpath.c_str(), LINUX_O_RDONLY, 0);
	// seems to work to here, can use this to create the new in-proc ELF loader
	
	// need to convert h into a FileDescriptor? Perhaps process should accept Handle
	// instead, file descriptors are just per-process mappings of Handle

	// need to maintain a reference to the Process object since
	// this will need a watcher thread to panic if init stops before service stops

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
}

//---------------------------------------------------------------------------

#pragma warning(pop)
