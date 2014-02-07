;//-----------------------------------------------------------------------------
;// Copyright (c) 2014 Michael G. Brehm
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
;//
;// The custom event logging code uses the error code's facility information
;// to determine the category of the event. Do not exceed 0xFF

FacilityNames=(
			Generic=0x00:FACILITY_GENERIC
			LinuxHost=0x01:FACILITY_LINUX_HOST
			)

;//--------------------------------------------------------------------------
;// Language Codes

LanguageNames=(English=0x0409:MSG00409)

;//--------------------------------------------------------------------------
;// Category Definitions
;//
;// These must match up to the facility codes defined above, if you want the
;// strings to show up in event viewer properly
;
;// #define your number of implemented categories here
;#define EVENTLOG_CATEGORY_COUNT		1
;

MessageIdTypedef=WORD

MessageId=0x00
SymbolicName=EVENTLOG_NAME
Language=English
ZukiSoft Virtual Machine Services
.

MessageId=0x01
SymbolicName=CATEGORY_LINUXHOST
Language=English
Linux Host
.

;//--------------------------------------------------------------------------
;// Error Definitions
;//--------------------------------------------------------------------------

MessageIdTypedef=HRESULT

;//--------------------------------------------------------------------------
;// GENERIC Error Codes (0x001-0x0FF)

MessageId=0x1
Severity=Informational
Facility=Generic
SymbolicName=GENERIC_INFORMATIONAL
Language=English
%1
.

MessageId=0x2
Severity=Warning
SymbolicName=GENERIC_WARNING
Language=English
%1
.

MessageId=0x3
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

;//--------------------------------------------------------------------------
;// LINUXHOST Error Codes (0x010-0x1FF)

MessageId=0x010
Severity=Informational
Facility=LinuxHost
SymbolicName=S_TEST
Language=English
Test Informational Message
.

MessageId=
Severity=Error
SymbolicName=E_INVALIDELFMAGIC
Language=English
The specified image does not contain the ELF header magic number and is invalid
.

MessageId=
Severity=Error
SymbolicName=E_UNEXPECTEDELFCLASS
Language=English
An unexpected ELF file class code of %d was detected in the ELF binary file header
.

MessageId=
Severity=Error
SymbolicName=E_UNEXPECTEDELFENCODING
Language=English
An unexpected ELF data encoding value was detected in the ELF binary file header
.

MessageId=
Severity=Error
SymbolicName=E_TRUNCATEDELFHEADER
Language=English
The ELF binary file header has been truncated and is invalid
.

MessageId=
Severity=Error
SymbolicName=E_UNKNOWNELFVERSION
Language=English
The ELF binary file header contains an unexpected version code or is of an unexpected length
.

MessageId=
Severity=Error
SymbolicName=E_INVALIDELFPROGRAMTABLE
Language=English
The ELF binary file header contains an invalid or corrupted program header table
.

MessageId=
Severity=Error
SymbolicName=E_INVALIDELFSECTIONTABLE
Language=English
The ELF binary file header contains an invalid or corrupted section header table
.

MessageId=
Severity=Error
SymbolicName=E_ELFSEGMENTFILEORDER
Language=English
The ELF binary file header lists segments in a non-sequential order
.

MessageId=0x101
Severity=Warning
SymbolicName=W_TEST
Language=English
Test Warning Message
.

MessageId=0x102
Severity=Error
SymbolicName=E_TEST
Language=English
Test Error Message
.

MessageId=0x105
Severity=Error
SymbolicName=E_ZLIB
Language=English
ZLIB DECOMPRESSION ERROR
.

MessageId=0x106
Severity=Error
SymbolicName=E_BZIP2
Language=English
BZIP2 DECOMPRESSION ERROR
.

MessageId=0x110
Severity=Warning
SymbolicName=W_LOADIMAGE_DECOMPRESS_ELF
Language=English
words here - Bad thing happened processing ELF image - attempting more methods
.

MessageId=0x111
Severity=Warning
SymbolicName=W_LOADIMAGE_DECOMPRESS_GZIP
Language=English
words here - Bad thing happened processing GZIP image - attempting more methods
.

MessageId=0x120
Severity=Error
SymbolicName=E_LOADIMAGE_DECOMPRESS
Language=English
Linux kernel image %1 could not be decompressed. More words here.
.

