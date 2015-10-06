;//-----------------------------------------------------------------------------
;// Copyright (c) 2015 Michael G. Brehm
;// 
;// Permission is hereby granted, free of charge, to any person obtaining a copy
;// of this software and associated documentation files (the "Software"), to deal
;// in the Software without restriction, including without limitation the rights
;// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;// copies of the Software, and to permit persons to whom the Software is
;// furnished to do so, subject to the following conditions:
;// 
;// The above copyright notice and this permission notice shall be included in all
;// copies or substantial portions of the Software.
;// 
;// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;// SOFTWARE.
;//-----------------------------------------------------------------------------

;//--------------------------------------------------------------------------
;// Facility Codes
FacilityNames=(
			Generic=0:FACILITY_GENERIC
			Common=1:FACILITY_COMMON
			Linux=2:FACILITY_LINUX
			Elf=3:FACILITY_ELF
			TaskState=4:FACILITY_TASKSTATE
			Process=5:FACILITY_PROCESS
			Thread=6:FACILITY_THREAD
			Service=7:FACILITY_SERVICE
			)

;//--------------------------------------------------------------------------
;// Language Codes
LanguageNames=(English=0x0409:MSG00409)

;//--------------------------------------------------------------------------
;// Error Definitions
;//--------------------------------------------------------------------------

MessageIdTypedef=HRESULT

;//--------------------------------------------------------------------------
;// GENERIC Error Codes

MessageId=1
Severity=Informational
Facility=Generic
SymbolicName=GENERIC_INFORMATIONAL
Language=English
%1
.

MessageId=
Severity=Warning
SymbolicName=GENERIC_WARNING
Language=English
%1
.

MessageId=
Severity=Error
SymbolicName=GENERIC_ERROR
Language=English
%1
.

MessageId=0x4
Severity=Error
SymbolicName=GENERIC_WIN32_ERROR
Language=English
%1.  Error Code %2 - %3
.

MessageId=
Severity=Error
SymbolicName=E_ARGUMENTNULL
Language=English
Parameter %1 value cannot be null.
.

MessageId=
Severity=Error
SymbolicName=E_ARGUMENTOUTOFRANGE
Language=English
Parameter %1 was out of the range of valid values.
.

;//----------------------------------------------------------------------------
;// Common
;//
;// Messages for custom exceptions thrown by common code

MessageId=1
Severity=Error
Facility=Common
SymbolicName=E_INDEXPOOL_EXHAUSTED
Language=English
The index pool has been exhausted; no more sequential index values can be allocated
.

MessageId=
Severity=Error
SymbolicName=E_DECOMPRESS_INIT
Language=English
The decompression stream cannot be initialized (method: %1)
.

MessageId=
Severity=Error
SymbolicName=E_DECOMPRESS_BADMAGIC
Language=English
The decompression stream magic number is invalid (method: %1)
.

MessageId=
Severity=Error
SymbolicName=E_DECOMPRESS_BADHEADER
Language=English
The decompression stream header is corrupt (method: %1)
.

MessageId=
Severity=Error
SymbolicName=E_DECOMPRESS_TRUNCATED
Language=English
The decompression stream ended prematurely (method: %1)
.

MessageId=
Severity=Error
SymbolicName=E_DECOMPRESS_CORRUPT
Language=English
The decompression stream data is corrupt (method: %1)
.

MessageId=
Severity=Error
SymbolicName=E_DECOMPRESS_TOOBIG
Language=English
The decompressed stream data is too big for this library (method: %1)
.

MessageId=
Severity=Error
SymbolicName=E_COMPRESSION_FORMAT
Language=English
Compressed file format %1 is not supported 
.

;//----------------------------------------------------------------------------
;// Linux
;//
;// Messages specific to the Linux environment

MessageId=1
Severity=Error
Facility=Linux
SymbolicName=E_LINUXEPERM 
Language=English
EPERM: Operation not permitted
.

MessageId=2
Severity=Error
SymbolicName=E_LINUXENOENT
Language=English
ENOENT: No such file or directory
.

MessageId=3
Severity=Error
SymbolicName=E_LINUXESRCH
Language=English
ESRCH: No such process
.

MessageId=4
Severity=Error
SymbolicName=E_LINUXEINTR
Language=English
EINTR: Interrupted system call
.

MessageId=5
Severity=Error
SymbolicName=E_LINUXEIO
Language=English
EIO: I/O error
.

MessageId=6
Severity=Error
SymbolicName=E_LINUXENXIO
Language=English
ENXIO: No such device or address
.

MessageId=7
Severity=Error
SymbolicName=E_LINUXE2BIG           
Language=English
E2BIG: Argument list too long
.

MessageId=8
Severity=Error
SymbolicName=E_LINUXENOEXEC         
Language=English
ENOEXEC: Exec format error
.

MessageId=9
Severity=Error
SymbolicName=E_LINUXEBADF           
Language=English
EBADF: Bad file number
.

MessageId=10
Severity=Error
SymbolicName=E_LINUXECHILD          
Language=English
ECHILD: No child processes
.

MessageId=11
Severity=Error
SymbolicName=E_LINUXEAGAIN          
Language=English
EAGAIN: Try again
.

MessageId=12
Severity=Error
SymbolicName=E_LINUXENOMEM          
Language=English
ENOMEM: Out of memory
.

MessageId=13
Severity=Error
SymbolicName=E_LINUXEACCES          
Language=English
EACCES: Permission denied
.

MessageId=14
Severity=Error
SymbolicName=E_LINUXEFAULT          
Language=English
EFAULT: Bad address
.

MessageId=15
Severity=Error
SymbolicName=E_LINUXENOTBLK         
Language=English
ENOTBLK: Block device required
.

MessageId=16
Severity=Error
SymbolicName=E_LINUXEBUSY           
Language=English
EBUSY: Device or resource busy
.

MessageId=17
Severity=Error
SymbolicName=E_LINUXEEXIST          
Language=English
EEXIST: File exists
.

MessageId=18
Severity=Error
SymbolicName=E_LINUXEXDEV           
Language=English
EXDEV: Cross-device link
.

MessageId=19
Severity=Error
SymbolicName=E_LINUXENODEV          
Language=English
ENODEV: No such device
.

MessageId=20
Severity=Error
SymbolicName=E_LINUXENOTDIR         
Language=English
ENOTDIR: Not a directory
.

MessageId=21
Severity=Error
SymbolicName=E_LINUXEISDIR          
Language=English
EISDIR: Is a directory
.

MessageId=22
Severity=Error
SymbolicName=E_LINUXEINVAL          
Language=English
EINVAL: Invalid argument
.

MessageId=23
Severity=Error
SymbolicName=E_LINUXENFILE          
Language=English
ENFILE: File table overflow
.

MessageId=24
Severity=Error
SymbolicName=E_LINUXEMFILE          
Language=English
EMFILE: Too many open files
.

MessageId=25
Severity=Error
SymbolicName=E_LINUXENOTTY          
Language=English
ENOTTY: Not a typewriter
.

MessageId=26
Severity=Error
SymbolicName=E_LINUXETXTBSY         
Language=English
ETXTBSY: Text file busy
.

MessageId=27
Severity=Error
SymbolicName=E_LINUXEFBIG           
Language=English
EFBIG: File too large
.

MessageId=28
Severity=Error
SymbolicName=E_LINUXENOSPC          
Language=English
ENOSPC: No space left on device
.

MessageId=29
Severity=Error
SymbolicName=E_LINUXESPIPE          
Language=English
ESPIPE: Illegal seek
.

MessageId=30
Severity=Error
SymbolicName=E_LINUXEROFS           
Language=English
EROFS: Read-only file system
.

MessageId=31
Severity=Error
SymbolicName=E_LINUXEMLINK          
Language=English
EMLINK: Too many links
.

MessageId=32
Severity=Error
SymbolicName=E_LINUXEPIPE          
Language=English
EPIPE: Broken pipe
.

MessageId=33
Severity=Error
SymbolicName=E_LINUXEDOM            
Language=English
EDOM: Math argument out of domain of func
.

MessageId=34
Severity=Error
SymbolicName=E_LINUXERANGE          
Language=English
ERANGE: Math result not representable
.

MessageId=35
Severity=Error
SymbolicName=E_LINUXEDEADLK         
Language=English
EDEADLK: Resource deadlock would occur
.

MessageId=36
Severity=Error
SymbolicName=E_LINUXENAMETOOLONG    
Language=English
ENAMETOOLONG: File name too long
.

MessageId=37
Severity=Error
SymbolicName=E_LINUXENOLCK          
Language=English
ENOLCK: No record locks available
.

MessageId=38
Severity=Error
SymbolicName=E_LINUXENOSYS          
Language=English
ENOSYS: Function not implemented
.

MessageId=39
Severity=Error
SymbolicName=E_LINUXENOTEMPTY       
Language=English
ENOTEMPTY: Directory not empty
.

MessageId=40
Severity=Error
SymbolicName=E_LINUXELOOP           
Language=English
ELOOP: Too many symbolic links encountered
.

MessageId=42
Severity=Error
SymbolicName=E_LINUXENOMSG          
Language=English
ENOMSG: No message of desired type
.

MessageId=43
Severity=Error
SymbolicName=E_LINUXEIDRM           
Language=English
EIDRM: Identifier removed
.

MessageId=44
Severity=Error
SymbolicName=E_LINUXECHRNG          
Language=English
ECHRNG: Channel number out of range
.

MessageId=45
Severity=Error
SymbolicName=E_LINUXEL2NSYNC        
Language=English
EL2NSYNC: Level 2 not synchronized
.

MessageId=46
Severity=Error
SymbolicName=E_LINUXEL3HLT          
Language=English
EL3HLT: Level 3 halted
.

MessageId=47
Severity=Error
SymbolicName=E_LINUXEL3RST          
Language=English
EL3RST: Level 3 reset
.

MessageId=48
Severity=Error
SymbolicName=E_LINUXELNRNG          
Language=English
ELNRNG: Link number out of range
.

MessageId=49
Severity=Error
SymbolicName=E_LINUXEUNATCH         
Language=English
EUNATCH: Protocol driver not attached
.

MessageId=50
Severity=Error
SymbolicName=E_LINUXENOCSI          
Language=English
ENOCSI: No CSI structure available
.

MessageId=51
Severity=Error
SymbolicName=E_LINUXEL2HLT          
Language=English
EL2HLT: Level 2 halted
.

MessageId=52
Severity=Error
SymbolicName=E_LINUXEBADE           
Language=English
EBADEL: Invalid exchange
.

MessageId=53
Severity=Error
SymbolicName=E_LINUXEBADR           
Language=English
EBADR: Invalid request descriptor
.

MessageId=54
Severity=Error
SymbolicName=E_LINUXEXFULL          
Language=English
EXFULL: Exchange full
.

MessageId=55
Severity=Error
SymbolicName=E_LINUXENOANO          
Language=English
ENOANO: No anode
.

MessageId=56
Severity=Error
SymbolicName=E_LINUXEBADRQC         
Language=English
EBADRQC: Invalid request code
.

MessageId=57
Severity=Error
SymbolicName=E_LINUXEBADSLT         
Language=English
EBADSLT: Invalid slot
.

MessageId=59
Severity=Error
SymbolicName=E_LINUXEBFONT          
Language=English
EBFONT: Bad font file format
.

MessageId=60
Severity=Error
SymbolicName=E_LINUXENOSTR          
Language=English
ENOSTR: Device not a stream
.

MessageId=61
Severity=Error
SymbolicName=E_LINUXENODATA         
Language=English
ENODATA: No data available
.

MessageId=62
Severity=Error
SymbolicName=E_LINUXETIME           
Language=English
ETIME: Timer expired
.

MessageId=63
Severity=Error
SymbolicName=E_LINUXENOSR           
Language=English
ENOSR: Out of streams resources
.

MessageId=64
Severity=Error
SymbolicName=E_LINUXENONET          
Language=English
ENONET: Machine is not on the network
.

MessageId=65
Severity=Error
SymbolicName=E_LINUXENOPKG          
Language=English
ENOPKG: Package not installed
.

MessageId=66
Severity=Error
SymbolicName=E_LINUXEREMOTE         
Language=English
EREMOTE: Object is remote
.

MessageId=67
Severity=Error
SymbolicName=E_LINUXENOLINK        
Language=English
ENOLINK: Link has been severed
.

MessageId=68
Severity=Error
SymbolicName=E_LINUXEADV            
Language=English
EADV: Advertise error
.

MessageId=69
Severity=Error
SymbolicName=E_LINUXESRMNT          
Language=English
ESRMNT: Srmount error
.

MessageId=70
Severity=Error
SymbolicName=E_LINUXECOMM           
Language=English
ECOMM: Communication error on send
.

MessageId=71
Severity=Error
SymbolicName=E_LINUXEPROTO          
Language=English
EPROTO: Protocol error
.

MessageId=72
Severity=Error
SymbolicName=E_LINUXEMULTIHOP       
Language=English
EMULTIHOP: Multihop attempted
.

MessageId=73
Severity=Error
SymbolicName=E_LINUXEDOTDOT         
Language=English
EDOTDOT: RFS specific error
.

MessageId=74
Severity=Error
SymbolicName=E_LINUXEBADMSG         
Language=English
EBADMSG: Not a data message
.

MessageId=75
Severity=Error
SymbolicName=E_LINUXEOVERFLOW       
Language=English
EOVERFLOW: Value too large for defined data type
.

MessageId=76
Severity=Error
SymbolicName=E_LINUXENOTUNIQ        
Language=English
ENOTUNIQ: Name not unique on network
.

MessageId=77
Severity=Error
SymbolicName=E_LINUXEBADFD          
Language=English
EBADFD: File descriptor in bad state
.

MessageId=78
Severity=Error
SymbolicName=E_LINUXEREMCHG         
Language=English
EREMCHG: Remote address changed
.

MessageId=79
Severity=Error
SymbolicName=E_LINUXELIBACC         
Language=English
ELIBACC: Can not access a needed shared library
.

MessageId=80
Severity=Error
SymbolicName=E_LINUXELIBBAD         
Language=English
ELIBBAD: Accessing a corrupted shared library
.

MessageId=81
Severity=Error
SymbolicName=E_LINUXELIBSCN         
Language=English
ELIBSCN: .lib section in a.out corrupted
.

MessageId=82
Severity=Error
SymbolicName=E_LINUXELIBMAX         
Language=English
ELIBMAX: Attempting to link in too many shared libraries
.

MessageId=83
Severity=Error
SymbolicName=E_LINUXELIBEXEC        
Language=English
ELIBEXEC: Cannot exec a shared library directly
.

MessageId=84
Severity=Error
SymbolicName=E_LINUXEILSEQ          
Language=English
EILSEQ: Illegal byte sequence
.

MessageId=85
Severity=Error
SymbolicName=E_LINUXERESTART        
Language=English
ERESTART: Interrupted system call should be restarted
.

MessageId=86
Severity=Error
SymbolicName=E_LINUXESTRPIPE        
Language=English
ESTRPIPE: Streams pipe error
.

MessageId=87
Severity=Error
SymbolicName=E_LINUXEUSERS          
Language=English
EUSERS: Too many users
.

MessageId=88
Severity=Error
SymbolicName=E_LINUXENOTSOCK        
Language=English
ENOTSOCK: Socket operation on non-socket
.

MessageId=89
Severity=Error
SymbolicName=E_LINUXEDESTADDRREQ    
Language=English
EDESTADDRREQ: Destination address required
.

MessageId=90
Severity=Error
SymbolicName=E_LINUXEMSGSIZE        
Language=English
EMSGSIZE: Message too long
.

MessageId=91
Severity=Error
SymbolicName=E_LINUXEPROTOTYPE      
Language=English
EPROTOTYPE: Protocol wrong type for socket
.

MessageId=92
Severity=Error
SymbolicName=E_LINUXENOPROTOOPT     
Language=English
ENOPROTOOPT: Protocol not available
.

MessageId=93
Severity=Error
SymbolicName=E_LINUXEPROTONOSUPPORT 
Language=English
EPROTONOSUPPORT: Protocol not supported
.

MessageId=94
Severity=Error
SymbolicName=E_LINUXESOCKTNOSUPPORT 
Language=English
ESOCKTNOSUPPORT: Socket type not supported
.

MessageId=95
Severity=Error
SymbolicName=E_LINUXEOPNOTSUPP      
Language=English
EOPNOTSUPP: Operation not supported on transport endpoint
.

MessageId=96
Severity=Error
SymbolicName=E_LINUXEPFNOSUPPORT    
Language=English
EPFNOSUPPORT: Protocol family not supported
.

MessageId=97
Severity=Error
SymbolicName=E_LINUXEAFNOSUPPORT    
Language=English
EAFNOSUPPORT: Address family not supported by protocol
.

MessageId=98
Severity=Error
SymbolicName=E_LINUXEADDRINUSE      
Language=English
EADDRINUSE: Address already in use
.

MessageId=99
Severity=Error
SymbolicName=E_LINUXEADDRNOTAVAIL   
Language=English
EADDRNOTAVAIL: Cannot assign requested address
.

MessageId=100
Severity=Error
SymbolicName=E_LINUXENETDOWN        
Language=English
ENETDOWN: Network is down
.

MessageId=101
Severity=Error
SymbolicName=E_LINUXENETUNREACH     
Language=English
ENETUNREACH: Network is unreachable
.

MessageId=102
Severity=Error
SymbolicName=E_LINUXENETRESET       
Language=English
ENETRESET: Network dropped connection because of reset
.

MessageId=103
Severity=Error
SymbolicName=E_LINUXECONNABORTED    
Language=English
ECONNABORTED: Software caused connection abort
.

MessageId=104
Severity=Error
SymbolicName=E_LINUXECONNRESET      
Language=English
ECONNRESET: Connection reset by peer
.

MessageId=105
Severity=Error
SymbolicName=E_LINUXENOBUFS         
Language=English
ENOBUFS: No buffer space available
.

MessageId=106
Severity=Error
SymbolicName=E_LINUXEISCONN         
Language=English
EISCONN: Transport endpoint is already connected
.

MessageId=107
Severity=Error
SymbolicName=E_LINUXENOTCONN        
Language=English
ENOTCONN: Transport endpoint is not connected
.

MessageId=108
Severity=Error
SymbolicName=E_LINUXESHUTDOWN       
Language=English
ESHUTDOWN: Cannot send after transport endpoint shutdown
.

MessageId=109
Severity=Error
SymbolicName=E_LINUXETOOMANYREFS    
Language=English
ETOOMANYREFS: Too many references: cannot splice
.

MessageId=110
Severity=Error
SymbolicName=E_LINUXETIMEDOUT       
Language=English
ETIMEDOUT: Connection timed out
.

MessageId=111
Severity=Error
SymbolicName=E_LINUXECONNREFUSED    
Language=English
ECONNREFUSED: Connection refused
.

MessageId=112
Severity=Error
SymbolicName=E_LINUXEHOSTDOWN       
Language=English
EHOSTDOWN: Host is down
.

MessageId=113
Severity=Error
SymbolicName=E_LINUXEHOSTUNREACH    
Language=English
EHOSTUNREACH: No route to host
.

MessageId=114
Severity=Error
SymbolicName=E_LINUXEALREADY        
Language=English
EALREADY: Operation already in progress
.

MessageId=115
Severity=Error
SymbolicName=E_LINUXEINPROGRESS     
Language=English
EINPROGRESS: Operation now in progress
.

MessageId=116
Severity=Error
SymbolicName=E_LINUXESTALE         
Language=English
ESTALE: Stale file handle
.

MessageId=117
Severity=Error
SymbolicName=E_LINUXEUCLEAN         
Language=English
EUCLEAN: Structure needs cleaning
.

MessageId=118
Severity=Error
SymbolicName=E_LINUXENOTNAM         
Language=English
ENOTNAM: Not a XENIX named type file
.

MessageId=119
Severity=Error
SymbolicName=E_LINUXENAVAIL         
Language=English
ENAVAIL: No XENIX semaphores available
.

MessageId=120
Severity=Error
SymbolicName=E_LINUXEISNAM          
Language=English
EISNAM: Is a named type file
.

MessageId=121
Severity=Error
SymbolicName=E_LINUXEREMOTEIO       
Language=English
EREMOTEIO: Remote I/O error
.

MessageId=122
Severity=Error
SymbolicName=E_LINUXEDQUOT          
Language=English
EDQUOT: Quota exceeded
.

MessageId=123
Severity=Error
SymbolicName=E_LINUXENOMEDIUM       
Language=English
ENOMEDIUM: No medium found
.

MessageId=124
Severity=Error
SymbolicName=E_LINUXEMEDIUMTYPE     
Language=English
EMEDIUMTYPE: Wrong medium type
.

MessageId=125
Severity=Error
SymbolicName=E_LINUXECANCELED       
Language=English
ECANCELED: Operation Canceled
.

MessageId=126
Severity=Error
SymbolicName=E_LINUXENOKEY          
Language=English
ENOKEY: Required key not available
.

MessageId=127
Severity=Error
SymbolicName=E_LINUXEKEYEXPIRED     
Language=English
EKEYEXPIRED: Key has expired
.

MessageId=128
Severity=Error
SymbolicName=E_LINUXEKEYREVOKED     
Language=English
EKEYREVOKED: Key has been revoked
.

MessageId=129
Severity=Error
SymbolicName=E_LINUXEKEYREJECTED    
Language=English
EKEYREJECTED: Key was rejected by service
.

MessageId=130
Severity=Error
SymbolicName=E_LINUXEOWNERDEAD      
Language=English
EOWNERDEAD: Owner died
.

MessageId=131
Severity=Error
SymbolicName=E_LINUXENOTRECOVERABLE 
Language=English
ENOTRECOVERABLE: State not recoverable
.

MessageId=132
Severity=Error
SymbolicName=E_LINUXERFKILL         
Language=English
ERFKILL: Operation not possible due to RF-kill
.

MessageId=133
Severity=Error
SymbolicName=E_LINUXEHWPOISON       
Language=English
EHWPOISON: Memory page has hardware error
.

;//----------------------------------------------------------------------------
;// Elf
;//
;// Messages specific to the parsing and loading of an ELF image

MessageId=1
Severity=Error
Facility=Elf
SymbolicName=E_ELFINVALIDMAGIC
Language=English
ELF image header does not contain the required magic number.
.

MessageId=
Severity=Error
SymbolicName=E_ELFINVALIDCLASS
Language=English
ELF image class %1!d! is not valid for execution on this platform.
.

MessageId=
Severity=Error
SymbolicName=E_ELFINVALIDENCODING
Language=English
ELF image encoding %1!d! is not valid for execution on this platform.
.

MessageId=
Severity=Error
SymbolicName=E_ELFINVALIDVERSION
Language=English
ELF image format version %1!d! is not supported.
.

MessageId=
Severity=Error
SymbolicName=E_ELFINVALIDTYPE
Language=English
Elf image type %1!d! is not valid for execution on this platform.
.

MessageId=
Severity=Error
SymbolicName=E_ELFINVALIDMACHINETYPE
Language=English
Elf image machine type %1!d! is not valid for execution on this platform.
.

MessageId=
Severity=Error
SymbolicName=E_ELFHEADERFORMAT
Language=English
ELF image header format is not supported.
.

MessageId=
Severity=Error
SymbolicName=E_ELFPROGHEADERFORMAT
Language=English
ELF image program header format is not supported.
.

MessageId=
Severity=Error
SymbolicName=E_ELFSECTHEADERFORMAT
Language=English
ELF image section header format is not supported.
.

MessageId=
Severity=Error
SymbolicName=E_ELFTRUNCATEDHEADER
Language=English
ELF image header has been truncated.
.

MessageId=
Severity=Error
SymbolicName=E_ELFIMAGETRUNCATED
Language=English
ELF image indicates a source data offset that lies beyond the end of the file.
.

MessageId=
Severity=Error
SymbolicName=E_ELFEXECUTABLESTACK
Language=English
ELF image specifies that the stack must be executable, which is not valid for this platform.
.

MessageId=
Severity=Error
SymbolicName=E_ELFRESERVEREGION
Language=English
Unable to reserve the virtual memory region required to load the ELF image.
.

MessageId=
Severity=Error
SymbolicName=E_ELFCOMMITSEGMENT
Language=English
Unable to commit the virtual memory required to load an ELF image segment.
.

MessageId=
Severity=Error
SymbolicName=E_ELFWRITESEGMENT
Language=English
Unable to write to the virtual memory region allocated for an ELF image segment.
.

MessageId=
Severity=Error
SymbolicName=E_ELFPROTECTSEGMENT
Language=English
Unable to set protection attributes on loaded ELF image segment.
.

MessageId=
Severity=Error
SymbolicName=E_ELFINVALIDINTERPRETER
Language=English
ELF image interpreter path is invalid or corrupt.
.

MessageId=
Severity=Error
SymbolicName=E_ELFARGUMENTSTOOBIG
Language=English
The amount of memory allocated (%1!lu! bytes) for an ELF startup information block exceeds the maximum size of %2!lu! bytes.
.

MessageId=
Severity=Error
SymbolicName=E_ELFWRITEARGUMENTS
Language=English
Unable to write ELF argument data to the allocated virtual memory region.
.

MessageId=
Severity=Error
SymbolicName=E_ELFARGUMENTSEXCEEDSTACK
Language=English
The amount of memory required (%1!lu! bytes) for the ELF process arguments exceeds the allocated stack size of %2!lu! bytes.
.

;//----------------------------------------------------------------------------
;// TaskState
;//
;// Messages specific to hosted process/thread task state

MessageId=1
Severity=Error
Facility=TaskState
SymbolicName=E_TASKSTATEUNSUPPORTEDCLASS
Language=English
The specified thread architecture class (%1!d!) is not supported.
.

MessageId=
Severity=Error
SymbolicName=E_TASKSTATEOVERFLOW
Language=English
The specified value (%1!d!) would overflow a 32-bit context task register.
.

MessageId=
Severity=Error
SymbolicName=E_TASKSTATEINVALIDLENGTH
Language=English
The length of the specified existing task state (%1!d! bytes) does not match the expected length for the thread architecture class (%2!d!).
.

MessageId=
Severity=Error
SymbolicName=E_TASKSTATEWRONGCLASS
Language=English
The specified thread architecture class (%1!d!) does not match that from which a task state was created (%2!d!).  This task state cannot be applied to the specified thread.
.

;//----------------------------------------------------------------------------
;// Process
;//
;// Messages specific to hosted process management

MessageId=1
Severity=Error
Facility=Process
SymbolicName=E_PROCESSInvalidX86Host
Language=English
The configured host application for 32-bit x86 processes is not a 32-bit application.
.

MessageId=
Severity=Error
SymbolicName=E_PROCESSInvalidX64Host
Language=English
The configured host application for 64-bit x86_64 processes is not a 64-bit application.
.

MessageId=
Severity=Error
SymbolicName=E_PROCESSInvalidThreadProc
Language=English
The remote thread entry point provided for a hosted process is invalid.
.

MessageId=
Severity=Error
SymbolicName=E_PROCESSInvalidStackSize
Language=English
The thread stack size virtual machine property is invalid.
.

MessageId=
Severity=Error
SymbolicName=E_PROCESSInvalidThreadTimeout
Language=English
The thread attach timeout virtual machine property is invalid.
.

MessageId=
Severity=Error
SymbolicName=E_PROCESSDuplicatePid
Language=English
The provided pid (%1!d!) cannot be inserted as a child for process (%2!d!); the pid has already been added.
.

MessageId=
Severity=Error
SymbolicName=E_PROCESSThreadTimeout
Language=English
Process (%1!d!) timed out waiting for a newly created native OS thread to become attached.
.

;//----------------------------------------------------------------------------
;// Thread
;//
;// Messages specific to hosted thread management

MessageId=1
Severity=Error
Facility=Thread
SymbolicName=E_THREADINVALIDSIGALTSTACK
Language=English
The pointer provided for a thread signal alternate stack is invalid.
.

;//----------------------------------------------------------------------------
;// Service
;//
;// Messages specific to the virtual machine service

MessageId=1
Severity=Error
Facility=Service
SymbolicName=E_INITRAMFSNOTFOUND
Language=English
The specified initial RAM file system (initramfs) archive %1 cannot be found or is not a file object
.

MessageId=
Severity=Error
SymbolicName=E_INITRAMFSEXTRACT
Language=English
An error occurred extracting the contents of the initial RAM file system (initramfs) archive %1: %2
.
