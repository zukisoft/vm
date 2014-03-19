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

#define RLIMIT_CPU			0	/* CPU time in sec */
#define RLIMIT_FSIZE		1	/* Maximum filesize */
#define RLIMIT_DATA			2	/* max data size */
#define RLIMIT_STACK		3	/* max stack size */
#define RLIMIT_CORE			4	/* max core file size */
#define RLIMIT_RSS			5	/* max resident set size */
#define RLIMIT_NPROC		6	/* max number of processes */
#define RLIMIT_NOFILE		7	/* max number of open files */
#define RLIMIT_MEMLOCK		8	/* max locked-in-memory address space */
#define RLIMIT_AS			9	/* address space limit */
#define RLIMIT_LOCKS		10	/* maximum file locks held */
#define RLIMIT_SIGPENDING	11	/* max number of pending signals */
#define RLIMIT_MSGQUEUE		12	/* maximum bytes in POSIX mqueues */
#define RLIMIT_NICE			13	/* max nice prio allowed to raise to 0-39 for nice level 19 .. -20 */
#define RLIMIT_RTPRIO		14	/* maximum realtime priority */
#define RLIMIT_RTTIME		15	/* timeout for RT tasks in us */
#define RLIM_NLIMITS		16

#define RLIM_INFINITY		(~0UL)

struct rlimit {

	unsigned long rlim_cur;
	unsigned long rlim_max;
};

// int ugetrlimit(int resource, struct rlimit *rlp);
//
// EBX	- int
// ECX	- struct rlimit*
// EDX
// ESI
// EDI
// EBP
//
int sys191_ugetrlimit(PCONTEXT context)
{
	MEMORY_BASIC_INFORMATION		stackInfo;		// Stack memory information

	// Get the pointer for the provided structure and fail if it's NULL
	struct rlimit* limit = reinterpret_cast<struct rlimit*>(context->Ecx);
	if(!limit) return -LINUX_EFAULT;

	// EBX = int resource
	switch(context->Ebx) {

		// RLIMIT_AS: Maximum size of a process' total available memory
		//
		case RLIMIT_AS:

			limit->rlim_cur = limit->rlim_max = (2UL << 30);		// 2GiB
			return 0;

		// RLIMIT_CORE / RLIMIT_FSIZE: Maximum allowable file sizes
		//
		case RLIMIT_CORE:
		case RLIMIT_FSIZE:

			limit->rlim_cur = limit->rlim_max = UINT32_MAX;
			return 0;

		// RLIMIT_STACK: Maximum size, in bytes of the process stack region
		//
		case RLIMIT_STACK:
			
			// Default to 1MB (Windows default) if this information cannot be queried
			limit->rlim_cur = limit->rlim_max = (1 << 20);

			// Get the base allocation address of the current stack pointer, and use that to get the 
			// original VirtualAlloc() region size.  NOTE: This can be done with GetThreadStackSize() in Windows 8+
			if(VirtualQuery(reinterpret_cast<void*>(context->Esp), &stackInfo, sizeof(MEMORY_BASIC_INFORMATION))) {
				if(VirtualQuery(stackInfo.AllocationBase, &stackInfo, sizeof(MEMORY_BASIC_INFORMATION)))
					limit->rlim_cur = limit->rlim_max = stackInfo.RegionSize;
			}

			return 0;

		// TODO: There are a lot more here

		// Unsupported resource
		default: return -LINUX_EINVAL;
	}

	return -LINUX_ENOSYS;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
