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
#include "Random.h"

#pragma warning(push, 4)

// Random::s_provider
//
HCRYPTPROV Random::s_provider = []() -> HCRYPTPROV {

	HCRYPTPROV provider;

	// Attempt to acquire a cryptography context; this handle will leak at process exit
	if(!CryptAcquireContext(&provider, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) return 0;
	return provider;
}();

//-----------------------------------------------------------------------------
// Random::Generate (static)
//
// Generates a random sequence of bytes and stores them into an output buffer
//
// Arguments:
//
//	buffer		- Output buffer
//	length		- Length of the output buffer, in bytes

void Random::Generate(void* buffer, size_t length)
{
	if(!buffer) throw Exception(E_ARGUMENTNULL, "buffer");
	if(length > MAXDWORD) throw Exception(E_ARGUMENTOUTOFRANGE, "length");

	// Load the output buffer with random data using the statically defined provider handle
	if(!CryptGenRandom(s_provider, static_cast<DWORD>(length), reinterpret_cast<uint8_t*>(buffer))) throw Win32Exception();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
