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

#ifndef __UAPI_H_
#define __UAPI_H_

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// types
//-----------------------------------------------------------------------------

typedef int					__kernel_pid_t;

typedef __kernel_pid_t		pid_t;

//-----------------------------------------------------------------------------
// errno-base.h
//-----------------------------------------------------------------------------

// moved to service IDL
//

//#define LINUX_EPERM              1              /* Operation not permitted */
//#define LINUX_ENOENT             2              /* No such file or directory */
//#define LINUX_ESRCH              3              /* No such process */
//#define LINUX_EINTR              4              /* Interrupted system call */
//#define LINUX_EIO                5              /* I/O error */
//#define LINUX_ENXIO              6              /* No such device or address */
//#define LINUX_E2BIG              7              /* Argument list too long */
//#define LINUX_ENOEXEC            8              /* Exec format error */
//#define LINUX_EBADF              9              /* Bad file number */
//#define LINUX_ECHILD            10              /* No child processes */
//#define LINUX_EAGAIN            11              /* Try again */
//#define LINUX_ENOMEM            12              /* Out of memory */
//#define LINUX_EACCES            13              /* Permission denied */
//#define LINUX_EFAULT            14              /* Bad address */
//#define LINUX_ENOTBLK           15              /* Block device required */
//#define LINUX_EBUSY             16              /* Device or resource busy */
//#define LINUX_EEXIST            17              /* File exists */
//#define LINUX_EXDEV             18              /* Cross-device link */
//#define LINUX_ENODEV            19              /* No such device */
//#define LINUX_ENOTDIR           20              /* Not a directory */
//#define LINUX_EISDIR            21              /* Is a directory */
//#define LINUX_EINVAL            22              /* Invalid argument */
//#define LINUX_ENFILE            23              /* File table overflow */
//#define LINUX_EMFILE            24              /* Too many open files */
//#define LINUX_ENOTTY            25              /* Not a typewriter */
//#define LINUX_ETXTBSY           26              /* Text file busy */
//#define LINUX_EFBIG             27              /* File too large */
//#define LINUX_ENOSPC            28              /* No space left on device */
//#define LINUX_ESPIPE            29              /* Illegal seek */
//#define LINUX_EROFS             30              /* Read-only file system */
//#define LINUX_EMLINK            31              /* Too many links */
//#define LINUX_EPIPE             32              /* Broken pipe */
//#define LINUX_EDOM              33              /* Math argument out of domain of func */
//#define LINUX_ERANGE            34              /* Math result not representable */

//-----------------------------------------------------------------------------
// errno.h
//-----------------------------------------------------------------------------

#define LINUX_EDEADLK           35              /* Resource deadlock would occur */
#define LINUX_ENAMETOOLONG      36              /* File name too long */
#define LINUX_ENOLCK            37              /* No record locks available */
//#define LINUX_ENOSYS            38              /* Function not implemented */
#define LINUX_ENOTEMPTY         39              /* Directory not empty */
#define LINUX_ELOOP             40              /* Too many symbolic links encountered */
#define LINUX_EWOULDBLOCK       LINUX_EAGAIN    /* Operation would block */
#define LINUX_ENOMSG            42              /* No message of desired type */
#define LINUX_EIDRM             43              /* Identifier removed */
#define LINUX_ECHRNG            44              /* Channel number out of range */
#define LINUX_EL2NSYNC          45              /* Level 2 not synchronized */
#define LINUX_EL3HLT            46              /* Level 3 halted */
#define LINUX_EL3RST            47              /* Level 3 reset */
#define LINUX_ELNRNG            48              /* Link number out of range */
#define LINUX_EUNATCH           49              /* Protocol driver not attached */
#define LINUX_ENOCSI            50              /* No CSI structure available */
#define LINUX_EL2HLT            51              /* Level 2 halted */
#define LINUX_EBADE             52              /* Invalid exchange */
#define LINUX_EBADR             53              /* Invalid request descriptor */
#define LINUX_EXFULL            54              /* Exchange full */
#define LINUX_ENOANO            55              /* No anode */
#define LINUX_EBADRQC           56              /* Invalid request code */
#define LINUX_EBADSLT           57              /* Invalid slot */
#define LINUX_EDEADLOCK         LINUX_EDEADLK
#define LINUX_EBFONT            59              /* Bad font file format */
#define LINUX_ENOSTR            60              /* Device not a stream */
#define LINUX_ENODATA           61              /* No data available */
#define LINUX_ETIME             62              /* Timer expired */
#define LINUX_ENOSR             63              /* Out of streams resources */
#define LINUX_ENONET            64              /* Machine is not on the network */
#define LINUX_ENOPKG            65              /* Package not installed */
#define LINUX_EREMOTE           66              /* Object is remote */
#define LINUX_ENOLINK           67              /* Link has been severed */
#define LINUX_EADV              68              /* Advertise error */
#define LINUX_ESRMNT            69              /* Srmount error */
#define LINUX_ECOMM             70              /* Communication error on send */
#define LINUX_EPROTO            71              /* Protocol error */
#define LINUX_EMULTIHOP         72              /* Multihop attempted */
#define LINUX_EDOTDOT           73              /* RFS specific error */
#define LINUX_EBADMSG           74              /* Not a data message */
#define LINUX_EOVERFLOW         75              /* Value too large for defined data type */
#define LINUX_ENOTUNIQ          76              /* Name not unique on network */
#define LINUX_EBADFD            77              /* File descriptor in bad state */
#define LINUX_EREMCHG           78              /* Remote address changed */
#define LINUX_ELIBACC           79              /* Can not access a needed shared library */
#define LINUX_ELIBBAD           80              /* Accessing a corrupted shared library */
#define LINUX_ELIBSCN           81              /* .lib section in a.out corrupted */
#define LINUX_ELIBMAX           82              /* Attempting to link in too many shared libraries */
#define LINUX_ELIBEXEC          83              /* Cannot exec a shared library directly */
#define LINUX_EILSEQ            84              /* Illegal byte sequence */
#define LINUX_ERESTART          85              /* Interrupted system call should be restarted */
#define LINUX_ESTRPIPE          86              /* Streams pipe error */
#define LINUX_EUSERS            87              /* Too many users */
#define LINUX_ENOTSOCK          88              /* Socket operation on non-socket */
#define LINUX_EDESTADDRREQ      89              /* Destination address required */
#define LINUX_EMSGSIZE          90              /* Message too long */
#define LINUX_EPROTOTYPE        91              /* Protocol wrong type for socket */
#define LINUX_ENOPROTOOPT       92              /* Protocol not available */
#define LINUX_EPROTONOSUPPORT   93              /* Protocol not supported */
#define LINUX_ESOCKTNOSUPPORT   94              /* Socket type not supported */
#define LINUX_EOPNOTSUPP        95              /* Operation not supported on transport endpoint */
#define LINUX_EPFNOSUPPORT      96              /* Protocol family not supported */
#define LINUX_EAFNOSUPPORT      97              /* Address family not supported by protocol */
#define LINUX_EADDRINUSE        98              /* Address already in use */
#define LINUX_EADDRNOTAVAIL     99              /* Cannot assign requested address */
#define LINUX_ENETDOWN          100             /* Network is down */
#define LINUX_ENETUNREACH       101             /* Network is unreachable */
#define LINUX_ENETRESET         102             /* Network dropped connection because of reset */
#define LINUX_ECONNABORTED      103             /* Software caused connection abort */
#define LINUX_ECONNRESET        104             /* Connection reset by peer */
#define LINUX_ENOBUFS           105             /* No buffer space available */
#define LINUX_EISCONN           106             /* Transport endpoint is already connected */
#define LINUX_ENOTCONN          107             /* Transport endpoint is not connected */
#define LINUX_ESHUTDOWN         108             /* Cannot send after transport endpoint shutdown */
#define LINUX_ETOOMANYREFS      109             /* Too many references: cannot splice */
#define LINUX_ETIMEDOUT         110             /* Connection timed out */
#define LINUX_ECONNREFUSED      111             /* Connection refused */
#define LINUX_EHOSTDOWN         112             /* Host is down */
#define LINUX_EHOSTUNREACH      113             /* No route to host */
#define LINUX_EALREADY          114             /* Operation already in progress */
#define LINUX_EINPROGRESS       115             /* Operation now in progress */
#define LINUX_ESTALE            116             /* Stale file handle */
#define LINUX_EUCLEAN           117             /* Structure needs cleaning */
#define LINUX_ENOTNAM           118             /* Not a XENIX named type file */
#define LINUX_ENAVAIL           119             /* No XENIX semaphores available */
#define LINUX_EISNAM            120             /* Is a named type file */
#define LINUX_EREMOTEIO         121             /* Remote I/O error */
#define LINUX_EDQUOT            122             /* Quota exceeded */
#define LINUX_ENOMEDIUM         123             /* No medium found */
#define LINUX_EMEDIUMTYPE       124             /* Wrong medium type */
#define LINUX_ECANCELED         125             /* Operation Canceled */
#define LINUX_ENOKEY            126             /* Required key not available */
#define LINUX_EKEYEXPIRED       127             /* Key has expired */
#define LINUX_EKEYREVOKED       128             /* Key has been revoked */
#define LINUX_EKEYREJECTED      129             /* Key was rejected by service */
#define LINUX_EOWNERDEAD        130             /* Owner died */
#define LINUX_ENOTRECOVERABLE   131             /* State not recoverable */
#define LINUX_ERFKILL           132             /* Operation not possible due to RF-kill */
#define LINUX_EHWPOISON         133             /* Memory page has hardware error */

#define LINUX_MAX_ERRNO			4095			/* Maximum defined errno value in kernel */

//-----------------------------------------------------------------------------
// mman-common.h
//-----------------------------------------------------------------------------

#define PROT_READ				0x1				/* page can be read */
#define PROT_WRITE				0x2				/* page can be written */
#define PROT_EXEC				0x4				/* page can be executed */
#define PROT_SEM				0x8				/* page may be used for atomic ops */
#define PROT_NONE				0x0				/* page can not be accessed */
#define PROT_GROWSDOWN			0x01000000		/* mprotect flag: extend change to start of growsdown vma */
#define PROT_GROWSUP			0x02000000		/* mprotect flag: extend change to end of growsup vma */

#define MAP_SHARED				0x01			/* Share changes */
#define MAP_PRIVATE				0x02			/* Changes are private */
#define MAP_TYPE				0x0f			/* Mask for type of mapping */
#define MAP_FIXED				0x10			/* Interpret addr exactly */
#define MAP_ANONYMOUS			0x20			/* don't use a file */
#define MAP_UNINITIALIZED		0x4000000		/* For anonymous mmap, memory could be uninitialized */

#define MS_ASYNC				1				/* sync memory asynchronously */
#define MS_INVALIDATE			2				/* invalidate the caches */
#define MS_SYNC					4				/* synchronous memory sync */

#define MADV_NORMAL				0				/* no further special treatment */
#define MADV_RANDOM				1				/* expect random page references */
#define MADV_SEQUENTIAL			2				/* expect sequential page references */
#define MADV_WILLNEED			3				/* will need these pages */
#define MADV_DONTNEED			4				/* don't need these pages */

#define MADV_REMOVE				9				/* remove these pages & resources */
#define MADV_DONTFORK			10				/* don't inherit across fork */
#define MADV_DOFORK				11				/* do inherit across fork */
#define MADV_HWPOISON			100				/* poison a page for testing */
#define MADV_SOFT_OFFLINE		101				/* soft offline page for testing */

#define MADV_MERGEABLE			12				/* KSM may merge identical pages */
#define MADV_UNMERGEABLE		13				/* KSM may not merge identical pages */
#define MADV_HUGEPAGE			14				/* Worth backing with hugepages */
#define MADV_NOHUGEPAGE			15				/* Not worth backing with hugepages */

#define MADV_DONTDUMP			16				/* Explicity exclude from the core dump, overrides the coredump filter bits */
#define MADV_DODUMP				17				/* Clear the MADV_NODUMP flag */

#define MAP_FILE				0				/* Compatibility flag */
#define MAP_HUGE_SHIFT			26
#define MAP_HUGE_MASK			0x3F

#define MAP_FAILED				((void*)-1)

//-----------------------------------------------------------------------------
// mman.h
//-----------------------------------------------------------------------------

#define MAP_GROWSDOWN			0x0100			/* stack-like segment */
#define MAP_DENYWRITE			0x0800			/* ETXTBSY */
#define MAP_EXECUTABLE			0x1000			/* mark it as an executable */
#define MAP_LOCKED				0x2000			/* pages are locked */
#define MAP_NORESERVE			0x4000			/* don't check for reservations */
#define MAP_POPULATE			0x8000			/* populate (prefault) pagetables */
#define MAP_NONBLOCK			0x10000			/* do not block on IO */
#define MAP_STACK				0x20000			/* give out an address that is best suited for process/thread stacks */
#define MAP_HUGETLB				0x40000			/* create a huge page mapping */

#define MCL_CURRENT				1				/* lock all current mappings */
#define MCL_FUTURE				2				/* lock all future mappings */

//-----------------------------------------------------------------------------
// arch/x86/include/uapi/asm/mman.h
//-----------------------------------------------------------------------------

#define MAP_32BIT				0x40			/* only give out 32bit addresses */

#define MAP_HUGE_2MB			(21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB			(30 << MAP_HUGE_SHIFT)

#define MAP_UNINITIALIZED		0x4000000

// Converts Linux PROT_XXX flags into Windows PAGE_XXX flags
//
inline static uint32_t ProtToPageFlags(uint32_t prot)
{
	switch(prot & (PROT_EXEC | PROT_WRITE | PROT_READ)) {

		case PROT_EXEC:								return PAGE_EXECUTE;
		case PROT_WRITE :							return PAGE_READWRITE;
		case PROT_READ :							return PAGE_READONLY;
		case PROT_EXEC | PROT_WRITE :				return PAGE_EXECUTE_READWRITE;
		case PROT_EXEC | PROT_READ :				return PAGE_EXECUTE_READ;
		case PROT_WRITE | PROT_READ :				return PAGE_READWRITE;
		case PROT_EXEC | PROT_WRITE | PROT_READ :	return PAGE_EXECUTE_READWRITE;
	}

	return PAGE_NOACCESS;
}

// Converts Linux PROT_XXX flags into Windows FILE_MAP_XXX flags
//
inline static uint32_t ProtToFileMapFlags(uint32_t prot)
{
	uint32_t flags = 0;
	
	if(prot & PROT_READ) flags |= FILE_MAP_READ;
	if(prot & PROT_WRITE) flags |= FILE_MAP_WRITE;
	if(prot & PROT_EXEC) flags |= FILE_MAP_EXECUTE;

	return flags;
}


typedef int pid_t;


struct user_desc {
	uint32_t entry_number;
	uint32_t base_addr;
	uint32_t limit;
	uint32_t seg_32bit:1;
	uint32_t contents:2;
	uint32_t read_exec_only:1;
	uint32_t limit_in_pages:1;
	uint32_t seg_not_present:1;
	uint32_t useable:1;
#ifdef _M_X64
	uint32_t lm:1;
#endif
};


//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif		// __UAPI_H_
