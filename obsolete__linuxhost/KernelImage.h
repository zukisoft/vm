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

#ifndef __KERNELIMAGE_H_
#define __KERNELIMAGE_H_
#pragma once

#include "BoyerMoore.h"					// Include BoyerMoore declarations
#include "BufferStreamReader.h"			// Include BufferStreamReader decls
#include "BZip2StreamReader.h"			// Include BZip2StreamReader decls
#include "ElfImage.h"					// Include ElfImage declarations
#include "Exception.h"					// Include Exception declarations
#include "File.h"						// Include File declarations
#include "GZipStreamReader.h"			// Include GZipStreamReader declarations
#include "Lz4StreamReader.h"			// Include Lz4StreamReader declarations
#include "LzopStreamReader.h"			// Include LzopStreamReader declarations
#include "MappedFile.h"					// Include MappedFile declarations
#include "MappedFileView.h"				// Include MappedFileView declarations
#include "MemoryRegion.h"				// Include MemoryRegion declarations
#include "StreamReader.h"				// Include StreamReader declarations
#include "Win32Exception.h"				// Include Win32Exception declarations
#include "XzStreamReader.h"				// Include XzStreamReader declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// KernelImage
//
// Loads the Linux system kernel image

class KernelImage
{
public:

	// Load
	//
	// Loads the specified Linux kernel image file
	static KernelImage* Load(LPCTSTR path);

private:

	// Instance Constructor
	//
	explicit KernelImage(ElfImage* image) { delete image; }

	//-------------------------------------------------------------------------
	// Private Member Functions

	static KernelImage* LoadCompressed(std::unique_ptr<StreamReader>& reader);

	//-------------------------------------------------------------------------
	// Member Variables

};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __KERNELIMAGE_H_