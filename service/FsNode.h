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

#ifndef __FSNODE_H_
#define __FSNODE_H_
#pragma once

#include "linux\stat.h"

#include "BlockDevice.h"
#include "CharacterDevice.h"
#include "PipeDevice.h"
#include "SocketDevice.h"



#pragma warning(push, 4)			

//-----------------------------------------------------------------------------

struct __fsnode_t {

	uint64_t		serialno;		// Node serial number
	uint32_t		type;			// Node type
	char*			name;			// Node name

	union {

		// S_IFBLK
		struct {

			BlockDevice*		device;
		
		} blockdev;

		// S_IFCHR
		struct {

			CharacterDevice*	device;
		
		} chardev;

		// S_IFREG
		struct {

			uint32_t temp;
		
		} file;

		// S_IFDIR
		struct {

		} directory;

		// S_IFLNK
		struct {	

		} link;

		// S_IFIFO
		struct {

			PipeDevice*			device;
		
		} pipedev;

		// S_IFSOCK
		struct {

			SocketDevice*		device;
		
		} socketdev;

	};
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FSNODE_H_
