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

#include "stdafx.h"						// Include project pre-compiled headers
#include "uapi.h"						// Include Linux UAPI declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

#define __NEW_UTS_LEN	64

struct new_utsname {

	char sysname[__NEW_UTS_LEN + 1];
	char nodename[__NEW_UTS_LEN + 1];
	char release[__NEW_UTS_LEN + 1];
	char version[__NEW_UTS_LEN + 1];
	char machine[__NEW_UTS_LEN + 1];
	char domainname[__NEW_UTS_LEN + 1];
 };

int sys122_newuname(PCONTEXT context)
{
	new_utsname* utsname = reinterpret_cast<new_utsname*>(context->Ebx);

	ZeroMemory(utsname, sizeof(new_utsname));
	lstrcpynA(utsname->sysname, "SYSNAME", __NEW_UTS_LEN);
	lstrcpynA(utsname->nodename, "NODENAME", __NEW_UTS_LEN);
	lstrcpynA(utsname->release, "RELEASE", __NEW_UTS_LEN);
	lstrcpynA(utsname->version, "VERSION", __NEW_UTS_LEN);
	lstrcpynA(utsname->machine, "i686", __NEW_UTS_LEN);
	lstrcpynA(utsname->domainname, "DOMAINNAME", __NEW_UTS_LEN);

	return 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
