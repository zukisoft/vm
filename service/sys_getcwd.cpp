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
#include "SystemCall.h"

#pragma warning(push, 4)

// LIMITATIONS
//
//	- chroot()ed processes have not been checked
//	- not sure what to do if the current directory has been deleted (unlinked)
//	- path building is not optimal, uses a collection and a reverse iterator

//-----------------------------------------------------------------------------
// sys_getcwd
//
// Gets the current working directory as an absolute path
//
// Arguments:
//
//	context		- SystemCall context object
//	buf			- Output buffer to receive the current working directory
//	size		- Length of the output buffer in bytes

__int3264 sys_getcwd(const SystemCall::Context* context, uapi::char_t* buf, size_t size)
{
	_ASSERTE(context);

	if(buf == nullptr) return -LINUX_EFAULT;

	try { 		
		
		SystemCall::Impersonation impersonation;

		// THIS NEEDS TO BE A VIRTUAL MACHINE CALL, TOO MUCH LOGIC FOR THE SYSTEM CALL

		std::vector<std::string> test;

		// Start at the current process working directory and continue working
		// backwards until a root node (node == parent) has been reached
		FileSystem::AliasPtr current = context->Process->WorkingDirectory;
		while(current->Parent != current) {

			// If the current node is a symbolic link, follow it to the target and loop again
			if(current->Node->Type == FileSystem::NodeType::SymbolicLink) {

				auto symlink = std::dynamic_pointer_cast<FileSystem::SymbolicLink>(current->Node);

				// if this throws, should it be "(unreachable)" -- see kernel code
				int loop = 0;	// For ELOOP detection
				current = symlink->Resolve(context->Process->RootDirectory, current, nullptr, 0, &loop);
				continue;
			}

			// Should never happen, but check for it regardless
			if(current->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

			// Push the next Alias name into the path building collection and move up to parent
			test.push_back(current->Name);
			current = current->Parent;
		}

		test.push_back("");

		std::string tododeleteme;
		for (auto iterator = test.begin(); iterator != test.end(); iterator++) {
			
			tododeleteme += "/";
			tododeleteme += *iterator;
		}

		if(tododeleteme.length() + 1 > size) throw LinuxException(LINUX_ERANGE);
		strncpy_s(buf, size, tododeleteme.c_str(), _TRUNCATE);
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	return 0;
}

// sys32_readlink
//
sys32_long_t sys32_getcwd(sys32_context_t context, sys32_char_t* buf, sys32_size_t size)
{
	return static_cast<sys32_long_t>(sys_getcwd(reinterpret_cast<SystemCall::Context*>(context), buf, size));
}

#ifdef _M_X64
// sys64_getcwd
//
sys64_long_t sys64_getcwd(sys64_context_t context, sys64_char_t* buf, sys64_sizeis_t size)
{
	return sys_getcwd(reinterpret_cast<SystemCall::Context*>(context), buf, static_cast<size_t>(size));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
