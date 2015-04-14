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

//-----------------------------------------------------------------------------
// VmService::CheckPermissions
//
// Demands read/write/execute permissions for a file system object
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be checked
//	flags		- Operation flags (AT_EACCESS, AT_SYMLINK_NO_FOLLOW)
//	mode		- Special MAY_READ, MAY_WRITE, MAY_EXECUTE flags

void VmService::CheckPermissions(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, 
	const uapi::char_t* path, int flags, uapi::mode_t mode)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Default behavior is to dereference symbolic links, sys_faccessat() can specify otherwise
	int resolveflags = ((flags & LINUX_AT_SYMLINK_NOFOLLOW) == LINUX_AT_SYMLINK_NOFOLLOW) ? LINUX_O_NOFOLLOW : 0;

	// TODO: support AT_EACCESS flag

	// Demand the requested permission(s) from the target file system node
	ResolvePath(root, base, path, resolveflags)->Node->DemandPermission(mode);
}

//-----------------------------------------------------------------------------
// VmService::AllocatePID
//
// Allocates a new process/thread identifier
//
// Arguments:
//
//	NONE

uapi::pid_t VmService::AllocatePID(void)
{
	return static_cast<uapi::pid_t>(m_pidpool.Allocate());
}

//-----------------------------------------------------------------------------
// VmService::CreateCharacterDevice
//
// Creates a file system character device object
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	mode		- Mode bitmask to use when creating the object
//	device		- Target device identifier

void VmService::CreateCharacterDevice(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, const uapi::char_t* path, 
	uapi::mode_t mode, uapi::dev_t device)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(root, base, splitter.Branch, 0);
	if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

	// Request a new character device node be created as a child of the resolved directory
	directory->CreateCharacterDevice(branch, splitter.Leaf, mode, device);
}

//-----------------------------------------------------------------------------
// VmService::CreateDirectory
//
// Creates a file system directory object
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	mode		- Mode bitmask to use if a new object is created

void VmService::CreateDirectory(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, 
	const uapi::char_t* path, uapi::mode_t mode)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(root, base, splitter.Branch, 0);
	if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
	
	auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

	directory->CreateDirectory(branch, splitter.Leaf, mode);
}

//-----------------------------------------------------------------------------
// VmService::CreateFile
//
// Creates a regular file object and returns a Handle instance
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	flags		- Flags indicating how the object should be opened
//	mode		- Mode bitmask to use if a new object is created

std::shared_ptr<FileSystem::Handle> VmService::CreateFile(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, 
	const uapi::char_t* path, int flags, uapi::mode_t mode)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// O_PATH and O_DIRECTORY cannot be used when creating a regular file object
	if((flags & LINUX_O_PATH) || (flags & LINUX_O_DIRECTORY)) throw LinuxException(LINUX_EINVAL);

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(root, base, splitter.Branch, flags);
	if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

	// Request a new regular file be created as a child of the resolved directory
	return directory->CreateFile(branch, splitter.Leaf, flags, mode);
}

//-----------------------------------------------------------------------------
// VmService::CloneProcess
//
// Clones an exising process
//
// Arguments:
//
//	TODO: Document when done

std::shared_ptr<Process> VmService::CloneProcess(const std::shared_ptr<Process>& process, uint32_t flags, void* tss, size_t tsslen)
{
	VirtualMachine::Properties hostprop;

	if(process->Architecture == Architecture::x86) hostprop = Properties::HostProcessBinary32;

#ifdef _M_X64
	else if(process->Architecture == Architecture::x86_64) hostprop = Properties::HostProcessBinary64;
#endif

	else throw Exception(E_INVALIDARG);				// <-- TODO: Exception

	// TODO: x64
	// this should move into the system call where the architecture is implied
	// Context::CreateTask<Architecture>(void*, length) ? something like that
	// task can be part of context

	std::shared_ptr<Process> child = process->Clone(flags, TaskState::FromExisting<Architecture::x86>(tss, tsslen));
	m_processes.insert(std::make_pair(child->ProcessId, child));

	child->Start();

	return child;
}

void VmService::RemoveProcess(uapi::pid_t pid)
{
	m_processes.erase(pid);
}

//-----------------------------------------------------------------------------
// VmService::CreateSymbolicLink
//
// Creates a file system symbolic link object
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	target		- Symbolic link target

void VmService::CreateSymbolicLink(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, 
	const uapi::char_t* path, const uapi::char_t* target)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(root, base, splitter.Branch, 0);
	if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
	
	auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

	directory->CreateSymbolicLink(branch, splitter.Leaf, target);
}

std::shared_ptr<Process> VmService::FindNativeProcess(DWORD nativepid)
{
	if(nativepid == m_initprocess->NativeProcessId) return m_initprocess;

	for(const auto& iterator : m_processes)
		if(nativepid == iterator.second->NativeProcessId) return iterator.second;

	return nullptr;
}

// GetAbsolutePath
void VmService::GetAbsolutePath(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& alias, uapi::char_t* path, size_t pathlen)
{
	std::vector<std::string> pathvec;

	if(path == nullptr) throw LinuxException(LINUX_EFAULT);
	if(pathlen == 0) throw LinuxException(LINUX_ERANGE);

	// TODO: not sure what to do if the current directory has been deleted (unlinked)

	// Start at the specified file system alias and continue working backwards until a root node is found
	FileSystem::AliasPtr current = alias;
	while(current->Parent != current) {

		// If the current node is a symbolic link, follow it to the target and loop again
		if(current->Node->Type == FileSystem::NodeType::SymbolicLink) {

			auto symlink = std::dynamic_pointer_cast<FileSystem::SymbolicLink>(current->Node);

			// if this throws, should it be "(unreachable)" -- see kernel code
			int loop = 0;	// For ELOOP detection
			current = symlink->Resolve(root, current, nullptr, 0, &loop);
			continue;
		}

		// Should never happen, but check for it regardless
		if(current->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

		// Push the next Alias name into the path building collection and move up to parent
		pathvec.push_back(current->Name);
		current = current->Parent;
	}

	pathvec.push_back("");

	std::string tododeleteme;
	for (auto iterator = pathvec.begin(); iterator != pathvec.end(); iterator++) {
			
		tododeleteme += "/";
		tododeleteme += *iterator;
	}

	if(tododeleteme.length() + 1 > pathlen) throw LinuxException(LINUX_ERANGE);
	strncpy_s(path, pathlen, tododeleteme.c_str(), _TRUNCATE);
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

const std::tstring& VmService::GetProperty(VirtualMachine::Properties id)
{
	return m_properties[id];
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

size_t VmService::GetProperty(VirtualMachine::Properties id, char_t* value, size_t length)
{
	const std::tstring& found = m_properties[id];

#ifndef _UNICODE
	strncpy_s(value, length, found.c_str(), _TRUNCATE);
	return strlen(value) + 1;
#else
	return WideCharToMultiByte(CP_UTF8, 0, found.data(), found.size(), value, length, nullptr, nullptr);
#endif
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
//	length		- Length of the destination buffer, in characters

size_t VmService::GetProperty(VirtualMachine::Properties id, wchar_t* value, size_t length)
{
	const std::tstring& found = m_properties[id];

#ifndef _UNICODE
	return MultiByteToWideChar(CP_ACP, 0, found.data(), found.size(), value, length);
#else
	wcsncpy_s(value, length, found.c_str(), _TRUNCATE);
	return wcslen(value) + 1;
#endif
}

//-----------------------------------------------------------------------------
// VmService::MountFileSystem
//
// Mounts a file system instance
//
// Arguments:
//
//	source		- Source device/object to be mounted
//	target		- Target directory on which to mount the file system
//	filesystem	- File system name
//	flags		- Standard mount options/flags
//	data		- Additional custom mounting data specific to filesystem
//	datalen		- Length of the additional custom mounting data

void VmService::MountFileSystem(const uapi::char_t* source, const uapi::char_t* target, const uapi::char_t* filesystem, uint32_t flags, void* data, size_t datalen)
{
	std::lock_guard<std::mutex> critsec(m_fslock);

	// Attempt to locate the filesystem by name in the collection
	auto result = m_availfs.find(filesystem);
	if(result == m_availfs.end()) throw LinuxException(LINUX_ENODEV);

	// Create the file system by passing the arguments into it's mount function
	auto mounted = result->second(source, flags, data, datalen);

	// Resolve the target alias and check that it's referencing a directory object
	auto alias = ResolvePath(m_rootfs->Root, m_rootfs->Root, target, 0);
	if(alias->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	// Overmount the target alias with the new file system's root node
	alias->Mount(mounted->Root->Node);

	// File system was successfully mounted, insert it into the member collection.
	// This will keep both the alias and the file system alive
	m_mounts.insert(std::make_pair(alias, mounted));
}

//-----------------------------------------------------------------------------
// VmService::OpenExecutable
//
// Opens an executable file system object and returns a Handle instance
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created

std::shared_ptr<FileSystem::Handle> VmService::OpenExecutable(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, 
	const uapi::char_t* path)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Attempt to resolve the file system object's name
	auto alias = ResolvePath(root, base, path, 0);
	
	// The only valid node type is a regular file object
	auto node = std::dynamic_pointer_cast<FileSystem::File>(alias->Node);
	if(node == nullptr) throw LinuxException(LINUX_ENOEXEC);

	// Create and return an executable handle for the file system object
	return node->OpenExec(alias);
}

//-----------------------------------------------------------------------------
// VmService::OpenFile
//
// Opens a file system object and returns a Handle instance
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	flags		- Flags indicating how the object should be opened
//	mode		- Mode bitmask to use if a new object is created

std::shared_ptr<FileSystem::Handle> VmService::OpenFile(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, 
	const uapi::char_t* path, int flags, uapi::mode_t mode)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// O_PATH filter -> Only O_CLOEXEC, O_DIRECTORY and O_NOFOLLOW are evaluated
	if(flags & LINUX_O_PATH) flags &= (LINUX_O_PATH | LINUX_O_CLOEXEC | LINUX_O_DIRECTORY | LINUX_O_NOFOLLOW);

	// O_CREAT | O_EXCL indicates that a regular file object must be created, call CreateFile() instead.
	if((flags & (LINUX_O_CREAT | LINUX_O_EXCL)) == (LINUX_O_CREAT | LINUX_O_EXCL)) return CreateFile(root, base, path, flags, mode);

	// O_CREAT indicates that if the object does not exist, a new regular file will be created
	else if((flags & LINUX_O_CREAT) == LINUX_O_CREAT) {

		PathSplitter splitter(path);				// Path splitter

		// Resolve the branch path to an Alias instance, must resolve to a Directory
		auto branch = ResolvePath(root, base, splitter.Branch, flags);
		if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

		// Ask the branch node to resolve the leaf, if that succeeds, just open it
		try { 
			
			auto leaf = ResolvePath(root, branch, splitter.Leaf, flags);
			return leaf->Node->Open(leaf, flags); 
		}
		
		catch(...) { /* DON'T CARE - KEEP GOING */ }

		// The leaf didn't exist (or some other bad thing happened), cast out the branch
		// as a Directory node and attempt to create the file instead
		auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
		if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

		return directory->CreateFile(branch, splitter.Leaf, flags, mode);
	}

	// Standard open, will throw exception if the object does not exist
	auto alias = ResolvePath(root, base, path, flags);
	return alias->Node->Open(alias, flags);
}

//-----------------------------------------------------------------------------
// VmService::ReadSymbolicLink
//
// Reads the target string from a file system symbolic link
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	buffer		- Target string output buffer
//	length		- Length of the target string output buffer

size_t VmService::ReadSymbolicLink(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, 
	const uapi::char_t* path, uapi::char_t* buffer, size_t length)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Ensure that the buffer pointer is not null and is at least one byte in length
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(length == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Find the desired symbolic link in the file system
	auto alias = ResolvePath(root, base, path, LINUX_O_NOFOLLOW);
	
	// The only valid node type is a symbolic link object; EINVAL if it's not
	auto symlink = std::dynamic_pointer_cast<FileSystem::SymbolicLink>(alias->Node);
	if(symlink == nullptr) throw LinuxException(LINUX_EINVAL);

	return symlink->ReadTarget(buffer, length);
}

//-----------------------------------------------------------------------------
// VmService::ReleasePID
//
// Releases a process/thread identifier allocated with AllocatePID
//
// Arguments:
//
//	pid			- PID to be released

void VmService::ReleasePID(uapi::pid_t pid)
{
	m_pidpool.Release(static_cast<int>(pid));
}

//-----------------------------------------------------------------------------
// VmService::ResolvePath
//
// Resolves a file system path to an alias instance
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path string to be resolved
//	flags		- Path resolution flags

std::shared_ptr<FileSystem::Alias> VmService::ResolvePath(const std::shared_ptr<FileSystem::Alias>& root, const std::shared_ptr<FileSystem::Alias>& base, 
	const uapi::char_t* path, int flags)
{
	int	symlinks = 0;							// Number of symbolic links encountered

	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	// If there is no path to consume, resolve to the base alias rather than
	// raising ENOENT.  This is a valid operation when a parent directory needs
	// to be resolved and it happens to be the base alias
	if(*path == 0) return base;

	// Determine if the path was absolute or relative and remove leading slashes
	bool absolute = (*path == '/');
	while(*path == '/') path++;

	// Start at either the root or the base depending on the path type, and ask
	// that node to resolve the now relative path string to an Alias instance
	auto current = (absolute) ? root : base;
	return current->Node->Resolve(root, current, path, flags, &symlinks);
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

void VmService::SetProperty(VirtualMachine::Properties id, const std::tstring& value)
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

void VmService::SetProperty(VirtualMachine::Properties id, const char_t* value)
{
	m_properties[id] = std::move(std::to_tstring(value));
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

void VmService::SetProperty(VirtualMachine::Properties id, const char_t* value, size_t length)
{
	m_properties[id] = std::move(std::to_tstring(value, static_cast<int>(length)));
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

void VmService::SetProperty(VirtualMachine::Properties id, const wchar_t* value)
{
	m_properties[id] = std::move(std::to_tstring(value));
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

void VmService::SetProperty(VirtualMachine::Properties id, const wchar_t* value, size_t length)
{
	m_properties[id] = std::move(std::to_tstring(value, static_cast<int>(length)));
}

void VmService::LoadInitialFileSystem(const std::shared_ptr<FileSystem::Alias>& target, const tchar_t* archivefile)
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
				FileSystem::HandlePtr p = CreateFile(target, target, path.c_str(), 0, file.Mode);
				std::vector<uint8_t> buffer(64 KiB);
				size_t read = file.Data.Read(buffer.data(), buffer.size());
				while(read > 0) {

					p->Write(buffer.data(), read);
					read = file.Data.Read(buffer.data(), buffer.size());
				}
			}
				break;

			case LINUX_S_IFDIR:
				CreateDirectory(target, target, path.c_str(), file.Mode);
				break;

			case LINUX_S_IFLNK:
			{
				std::vector<char> buffer(file.Data.Length + 1);
				file.Data.Read(buffer.data(), buffer.size());
				std::string linktarget = std::to_string(buffer.data());
				CreateSymbolicLink(target, target, path.c_str(), linktarget.c_str());
			}
				break;
				
			// S_IFCHR - Create a character device node
			case LINUX_S_IFCHR:
				CreateCharacterDevice(target, target, path.c_str(), file.Mode, CreateDeviceId(file.ReferencedDeviceMajor, file.ReferencedDeviceMinor));
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
		// NATIVE JOB
		//
		m_nativejob = NativeHandle::FromHandle(CreateJobObject(nullptr, nullptr));

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

		SetProperty(Properties::HostProcessBinary32, process_host_32bit);
#ifdef _M_X64
		SetProperty(Properties::HostProcessBinary64, process_host_64bit);
#endif
		SetProperty(Properties::ThreadStackSize, "2097152");	// 2 MiB
		SetProperty(Properties::ThreadAttachTimeout, "15000");	// 15 seconds

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

		// temporary -- mount the root file system; this needs to actually be something
		// specified by the options for the service instance
		m_rootfs = RootFileSystem::Mount(nullptr, 0, nullptr, 0);

		// add filesystems
		// no need to lock, service has not gone multi-threaded yet; no RPC listener
		m_availfs.insert(std::make_pair("hostfs", HostFileSystem::Mount));
		m_availfs.insert(std::make_pair("procfs", std::bind(&VmService::MountProcFileSystem, this, _1, _2, _3)));
		//m_availfs.insert(std::make_pair("rootfs", RootFileSystem::Mount));		// <--- FIX ME
		m_availfs.insert(std::make_pair("tmpfs", TempFileSystem::Mount));

		//MountFileSystem("D:\\Linux Stuff\\android-l-preview_r2-x86\\root", "/", "hostfs", 0, nullptr, 0);
		MountFileSystem("tmpfs", "/", "tmpfs", 0, nullptr, 0);

		// ??? PROCFS / SYSFS
		//
		// This should be handled by init, but it can be tested here once PROCFS does something
		//CreateDirectory(m_rootfs->Root, m_rootfs->Root, "/proc", 0);
		//MountFileSystem(nullptr, "/proc", "procfs", 0, nullptr, 0);

		//
		// INITRAMFS
		//

		// Use the existence of the setting to determine if initramfs feature should be used
		std::tstring initramfs = vm_initramfs;
		if(initramfs.length()) {

			// Ensure that the initramfs file actually exists on the host file system
			if(!File::Exists(initramfs.c_str())) throw Exception(E_INITRAMFSNOTFOUND, initramfs.c_str());

			// Attempt to extract the contents of the initramfs into the current root file system
			try { LoadInitialFileSystem(m_rootfs->Root, initramfs.c_str()); }
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

		SetProperty(Properties::HostProcessArguments, syscall32_listener::GetBindingString(this->InstanceID));

#ifdef _M_X64
		// x64 builds also register the 64-bit system calls interface
		syscall64_listener::Register(RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY);
		syscall64_listener::AddObject(this->InstanceID);
#endif
	} 

	// Win32Exception and Exception can be translated into a ServiceException
	catch(Win32Exception& ex) { throw ServiceException(static_cast<DWORD>(ex.Code)); }
	catch(Exception& ex) { throw ServiceException(ex.HResult); }

	//
	// TEST: DEFAULT NAMESPACE
	//
	auto defaultns = Namespace::Create();

	//
	// LAUNCH INIT
	//
	std::string initpath = std::to_string(vm_initpath);
	const uapi::char_t* args[] = { initpath.c_str(), "First Argument", "Second Argument", nullptr };

	// TESTING - NEW SESSION/GROUP/PROCESS RELATIONSHIPS (overcomes Process::Spawn)
	// it would be nice to return the Process instance, but this works too I guess

	auto rootpid = defaultns->Pids->Allocate();
	// try/catch/release pid? why bother, whole VM dies if init can't be loaded
	auto exec = Executable::FromFile(initpath.c_str(), args, nullptr, m_rootfs->Root, m_rootfs->Root);
	auto rootsession = Session::FromExecutable(shared_from_this(), rootpid, defaultns, exec);

	// Get the init process from the root session and root process group, this is inefficient but quick enough
	m_initprocess = rootsession->ProcessGroup[rootpid]->Process[rootpid];

	// TODO: NEED INITIAL ENVIRONMENT
	// note: no parent

	// following line is OBE from Session/ProcessGroup stuff, remove it
	//m_initprocess = Process::Spawn(shared_from_this(), 1, nullptr, initpath.c_str(), args, nullptr, m_rootfs->Root, m_rootfs->Root);
	
	// stdout/stderr test
	m_initprocess->AddHandle(1, OpenFile(m_rootfs->Root, m_rootfs->Root, "/dev/console", LINUX_O_RDWR, 0));
	m_initprocess->AddHandle(2, OpenFile(m_rootfs->Root, m_rootfs->Root, "/dev/console", LINUX_O_RDWR, 0));

	m_initprocess->Start();

	// TODO: NOTE: The message loop, and therefore signals will not be available in the process

	// TODO: EXCEPTION HANDLING FOR INIT PROCESS -- DIFFERENT?
	// TODO: MONITOR INIT PROCESS - PANIC IF IT TERMINATES BEFORE SERVICE IS STOPPED

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
	// TODO: TESTING
	TerminateJobObject(m_nativejob->Handle, static_cast<UINT>(E_UNEXPECTED));

	// TODO: TESTING
	m_initprocess.reset();

#ifdef _M_X64
	// Remove the 64-bit system calls RPC interface
	syscall64_listener::RemoveObject(this->InstanceID);
	syscall64_listener::Unregister(true);
#endif

	// Remove the 32-bit system calls RPC interface
	syscall32_listener::RemoveObject(this->InstanceID);
	syscall32_listener::Unregister(true);
}

//-----------------------------------------------------------------------------
// VmService::SpawnProcess
//
// Spawns a new process.  A spawned process will have no parent and will use
// the file system root directory for both its working and root directories
//
// Arguments:
//
//	filename		- Path to the binary used to spawn the process
//	argv			- Process command line arguments
//	envp			- Process environment variables

std::shared_ptr<Process> VmService::SpawnProcess(const char_t* filename, const char_t* const* argv, const char_t* const* envp)
{
	(filename);
	(argv);
	(envp);
	//uapi::pid_t pid = m_pidpool.Allocate();			// Allocate a PID for the new process

	//try { 
	//	
	//	// Attempt to spawn the new process
	//	auto process = Process::FromFile(shared_from_this(), pid, filename, argv, envp, m_rootfs->Root, m_rootfs->Root);

	//	// Take a writer lock on the process collection and add the new process to it.  The lock can be taken just
	//	// prior to inserting the item since the key (PID) is already guaranteed to be unique via the pool class
	//	process_map_lock_t::scoped_lock writer(m_processeslock);
	//	m_processes.insert(std::make_pair(pid, process));

	//	return process;								// Return the newly created process instance
	//}

	//catch(...) { m_pidpool.Release(pid); throw; }

	return nullptr;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
