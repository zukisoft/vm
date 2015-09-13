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
#include "Binary.h"

#include "ElfBinary.h"
#include "Executable.h"
#include "Host.h"
#include "LinuxException.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Binary::Load (static)
//
// Loads an executable binary image into a host instance
//
// Arguments:
//
//	host		- Host instance in which to load the binary
//	executable	- Executable instance to be loaded

std::unique_ptr<Binary> Binary::Load(Host* host, Executable const* executable)
{
	switch(executable->Format) {

		// ELF --> ElfBinary
		//
		case BinaryFormat::ELF: return ElfBinary::Load(host, executable);
	}
	
	// Unknown or unsupported binary format
	throw LinuxException{ LINUX_ENOEXEC };
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
