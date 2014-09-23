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

#ifndef __VIRTUALMACHINE_H_
#define __VIRTUALMACHINE_H_
#pragma once

#include <memory.h>
#include "VmFileSystem.h"
#include "VmSettings.h"
#include "VmSystemLog.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// VirtualMachine
//
// Interface implemented by the main service, provides a coherent view of all
// the various VM subsystems so each can interoperate with one another

struct __declspec(novtable) VirtualMachine
{
	// FileSystem
	//
	// Accesses the virtual machine's file system instance
	__declspec(property(get=getFileSystem)) std::unique_ptr<VmFileSystem>& FileSystem;
	virtual std::unique_ptr<VmFileSystem>& getFileSystem(void) = 0;

	// Settings
	//
	// Accesses the virtual machine's settings instance
	__declspec(property(get=getSettings)) std::unique_ptr<VmSettings>& Settings;
	virtual std::unique_ptr<VmSettings>& getSettings(void) = 0;
	
	// SystemLog
	//
	// Accesses the virtual machine's system log instance
	__declspec(property(get=getSystemLog)) std::unique_ptr<VmSystemLog>& SystemLog;
	virtual std::unique_ptr<VmSystemLog>& getSystemLog(void) = 0;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALMACHINE_H_
