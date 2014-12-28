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

// g_rpccontext
//
// Global RPC context handle to the system calls server
extern sys32_context_t g_rpccontext;

//-----------------------------------------------------------------------------
// TraceMessage
//
// Sends a trace message back to the RPC server, OutputDebugString is flaky
// when combined with a vectored exception handler, this resolves that by 
// allowing the service to do whatever it chooses with the message
//
// Arguments:
//
//	message			- ANSI trace message
//	length			- Length of the trace message, in characters

void TraceMessage(const char_t* message, size_t length)
{
	// Nothing much to do, just send the message to the service
	sys32_trace(g_rpccontext, const_cast<sys32_char_t*>(message), length);
}

//-----------------------------------------------------------------------------
