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

#include "stdafx.h"
#include <vm.service.h>

#pragma warning(push, 4)			

//-----------------------------------------------------------------------------
// open

__int3264 rpc005_open(handle_t client, charptr_t pathname, int32_t flags, uapi::mode_t mode, fsobject_t* fsobject)
{
	UNREFERENCED_PARAMETER(client);
	UNREFERENCED_PARAMETER(flags);
	UNREFERENCED_PARAMETER(mode);

	fsobject->fshandle = 0;
	fsobject->objecttype = FSOBJECT_PHYSICAL;

	size_t len = (11 + strlen(pathname) + 1);
	fsobject->physical.ospath = (wcharptr_t)midl_user_allocate(len * sizeof(wchar_t));

	wcscpy_s(fsobject->physical.ospath, len, L"D:\\android");
	MultiByteToWideChar(CP_UTF8, 0, pathname, -1, &fsobject->physical.ospath[10], static_cast<int>(len-10));

	wchar_t* iterator = fsobject->physical.ospath;
	while(*iterator) {
		if(*iterator == '/') *iterator = '\\';
		iterator++;
	}

	return 0;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
