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
FacilityNames=(
			Generic=0x00:FACILITY_GENERIC
			ElfImage=0x01:FACILITY_ELFIMAGE
			ElfLoader=0x02:FACILITY_ELF_LOADER
			)

;//--------------------------------------------------------------------------
;// Language Codes
LanguageNames=(English=0x0409:MSG00409)

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
;// ElfImage
;//
;// Messages specific to the parsing and loading of an ELF image

MessageId=0x100
Severity=Error
Facility=ElfImage
SymbolicName=E_TRUNCATEDELFHEADER
Language=English
ELF image header has been truncated.
.

MessageId=
Severity=Error
SymbolicName=E_INVALIDELFMAGIC
Language=English
ELF image header does not contain the required magic number.
.

MessageId=
Severity=Error
SymbolicName=E_INVALIDELFCLASS
Language=English
ELF image class %1!d! is not valid for execution on this platform.
.

MessageId=
Severity=Error
SymbolicName=E_INVALIDELFENCODING
Language=English
ELF image encoding %1!d! is not valid for execution on this platform.
.

MessageId=
Severity=Error
SymbolicName=E_INVALIDELFVERSION
Language=English
ELF image format version %1!d! is not supported.
.

MessageId=
Severity=Error
SymbolicName=E_INVALIDELFTYPE
Language=English
Elf image type %1!d! is not valid for execution on this platform.
.

MessageId=
Severity=Error
SymbolicName=E_INVALIDELFMACHINETYPE
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
SymbolicName=E_ELFIMAGETRUNCATED
Language=English
ELF image indicates a source data offset that lies beyond the end of the file.
.

MessageId=
Severity=Error
SymbolicName=E_INVALIDINTERPRETER
Language=English
ELF image interpreter path is invalid or corrupt.
.

MessageId=
Severity=Error
SymbolicName=E_EXECUTABLESTACKFLAG
Language=English
ELF image specifies that the stack must be executable, which is not valid for this platform.
.

MessageId=
Severity=Error
SymbolicName=E_RESERVEIMAGEREGION
Language=English
Unable to reserve the virtual memory region required to load the ELF image.
.

MessageId=
Severity=Error
SymbolicName=E_COMMITIMAGESEGMENT
Language=English
Unable to commit the virtual memory required to load an ELF image segment.
.

MessageId=
Severity=Error
SymbolicName=E_PROTECTIMAGESEGMENT
Language=English
Unable to set protection attributes on loaded ELF image segment.
.

MessageId=
Severity=Error
SymbolicName=E_LOADELFIMAGEFAILED
Language=English
Cannot load ELF image %1.
.

MessageId=
Severity=Error
SymbolicName=E_NULLELFENTRYPOINT
Language=English
ELF image cannot be executed as no entry point has been specified
.

MessageId=
Severity=Error
SymbolicName=E_ARGUMENTVECTORALIGNMENT
Language=English
Internal Error: The ELF argument vector has not been aligned properly for the stack
.

