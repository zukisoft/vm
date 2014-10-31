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

// sys_readlink
//
// words
__int3264 sys_readlink(const SystemCall::Context* context, const uapi::char_t* pathname, void* buf, size_t bufsiz)
{
	_ASSERTE(context);
	(pathname);
	(buf);
	(bufsiz);

	try { 		SystemCall::Impersonation impersonation;
		return -1; /*context->VirtualMachine->DOUNAME(context->Process, buf);*/ }
	catch(...) { return SystemCall::TranslateException(std::current_exception()); }
}

// sys32_readlink
//
sys32_long_t sys32_readlink(sys32_context_t context, const sys32_char_t* pathname, sys32_char_t* buf, sys32_size_t bufsiz)
{
	return static_cast<sys32_long_t>(sys_readlink(reinterpret_cast<SystemCall::Context*>(context), pathname, buf, bufsiz));
}

#ifdef _M_X64
// sys64_readlink
//
sys64_long_t sys64_readlink(sys64_context_t context, const sys64_char_t* pathname, sys64_char_t* buf, sys64_sizeis_t bufsiz)
{
	return sys_readlink(reinterpret_cast<SystemCall::Context*>(context), pathname, buf, static_cast<size_t>(bufsiz));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
