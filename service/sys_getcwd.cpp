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

// sys_getcwd
//
// Gets the current working directory
__int3264 sys_getcwd(const SystemCall::Context* context, uapi::char_t* buf, size_t size)
{
	_ASSERTE(context);

	try { 		
		
		SystemCall::Impersonation impersonation;

		// Here's how I think this will need to work with the implementation I chose.
		// Start at the current alias and work backwards towards the root node.
		// If a symbolic link is detected, resolve that symlink and go again

		//std::vector<std::string> test;
		//FileSystem::AliasPtr current = context->Process->WorkingDirectory;
		//if(current->Node->Type == FileSystem::NodeType::SymbolicLink) {

		//	auto symlink = std::dynamic_pointer_cast<FileSystem::SymbolicLink>(current->Node);
		//	int loop = 0;
		//	current = symlink->Resolve(context->Process->RootDirectory, current, nullptr, 0, &loop);
		//}

		//// Need to deal with chroot() somehow, can't back up past the process root

		//if(current->Parent == current) test.push_back("/");
		//else test.push_back(std::string(current->Name));

		//current = current->Parent;

		strncpy_s(buf, size, "/", size);
		//memset(buf, 0, size);
		return 0;
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }
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
