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

#ifndef __BOYERMOORE_H_
#define __BOYERMOORE_H_
#pragma once

#include "Exception.h"

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// BoyerMoore
//
// Implementation of the Boyer-Moore search string algorithm
// See http://en.wikipedia.org/wiki/Boyer%E2%80%93Moore_string_search_algorithm

class BoyerMoore
{
public:

	// Search
	//
	// Executes a binary pattern search using the Boyer-Moore algorithm
	static void* Search(void* haystack, size_t haystacklen, void* needle, size_t needlelen);

private:

	BoyerMoore();
	BoyerMoore(const BoyerMoore&);
	BoyerMoore& operator=(const BoyerMoore&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// Boyer-Moore Algorithm Functions
	//
	static uint8_t* BoyerMooreSearch(uint8_t *string, uint32_t stringlen, uint8_t *pat, uint32_t patlen);
	static int		IsPrefix(uint8_t *word, int wordlen, int pos);
	static void		MakeDelta1(int *delta1, uint8_t *pat, int32_t patlen);
	static void		MakeDelta2(int *delta2, uint8_t *pat, int32_t patlen);
	static int		SuffixLength(uint8_t *word, int wordlen, int pos);
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __BOYERMOORE_H_
