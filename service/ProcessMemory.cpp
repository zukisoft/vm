//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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
#include "ProcessMemory.h"

#pragma warning(push, 4)

//
// PROCESSMEMORY::ALLOCATIONFLAGS
//

// ProcessMemory::AllocationFlags::None (static)
//
ProcessMemory::AllocationFlags const ProcessMemory::AllocationFlags::None{ 0x00 };

// ProcessMemory::AllocationFlags::TopDown (static)
//
ProcessMemory::AllocationFlags const ProcessMemory::AllocationFlags::TopDown{ 0x01 };

//
// PROCESSMEMORY::PROTECTION
//

// ProcessMemory::Protection::Execute (static)
//
ProcessMemory::Protection const ProcessMemory::Protection::Execute{ 0x01 };

// ProcessMemory::Protection::Guard (static)
//
ProcessMemory::Protection const ProcessMemory::Protection::Guard{ 0x80 };

// ProcessMemory::Protection::None (static)
//
ProcessMemory::Protection const ProcessMemory::Protection::None{ 0x00 };

// ProcessMemory::Protection::Read (static)
//
ProcessMemory::Protection const ProcessMemory::Protection::Read{ 0x02 };

// ProcessMemory::Protection::Write (static)
//
ProcessMemory::Protection const ProcessMemory::Protection::Write{ 0x04 };

//-----------------------------------------------------------------------------

#pragma warning(pop)