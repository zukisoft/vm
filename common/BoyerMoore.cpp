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

#include "stdafx.h"					// Include project pre-compiled headers
#include "BoyerMoore.h"				// Include BoyerMoore class declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// BoyerMoore::BoyerMooreSearch (private, static)
//
// Executes a Boyer-Moore pattern search
// See http://en.wikipedia.org/wiki/Boyer%E2%80%93Moore_string_search_algorithm

uint8_t* BoyerMoore::BoyerMooreSearch(uint8_t *string, uint32_t stringlen, uint8_t *pat, uint32_t patlen) 
{
    uint32_t i;
    int delta1[UINT8_MAX];
    int *delta2 = (int *)malloc(patlen * sizeof(int));
    MakeDelta1(delta1, pat, patlen);
    MakeDelta2(delta2, pat, patlen);
 
    i = patlen-1;
    while (i < stringlen) {
        int j = patlen-1;
        while (j >= 0 && (string[i] == pat[j])) {
            --i;
            --j;
        }
        if (j < 0) {
            free(delta2);
            return (string + i+1);
        }
 
        i += max(delta1[string[i]], delta2[j]);
    }
    free(delta2);
    return NULL;
}

//-----------------------------------------------------------------------------
// BoyerMoore::IsPrefix (private, static)
//
// true if the suffix of word starting from word[pos] is a prefix of word

int BoyerMoore::IsPrefix(uint8_t *word, int wordlen, int pos) 
{
    int i;
    int suffixlen = wordlen - pos;
    // could also use the strncmp() library function here
    for (i = 0; i < suffixlen; i++) {
        if (word[i] != word[pos+i]) {
            return 0;
        }
    }
    return 1;
}
 
//-----------------------------------------------------------------------------
// BoyerMoore::MakeDelta1 (private, static)
//
// delta1 table: delta1[c] contains the distance between the last
// character of pat and the rightmost occurrence of c in pat.
// If c does not occur in pat, then delta1[c] = patlen.
// If c is at string[i] and c != pat[patlen-1], we can
// safely shift i over by delta1[c], which is the minimum distance
// needed to shift pat forward to get string[i] lined up 
// with some character in pat.
// this algorithm runs in alphabet_len+patlen time.

void BoyerMoore::MakeDelta1(int *delta1, uint8_t *pat, int32_t patlen) 
{
    int i;
    for (i=0; i < UINT8_MAX; i++) {
        delta1[i] = patlen;
    }
    for (i=0; i < patlen-1; i++) {
        delta1[pat[i]] = patlen-1 - i;
    }
}
 
//-----------------------------------------------------------------------------
// BoyerMoore::MakeDelta2 (private, static)
//
// delta2 table: given a mismatch at pat[pos], we want to align 
// with the next possible full match could be based on what we
// know about pat[pos+1] to pat[patlen-1].
//
// In case 1:
// pat[pos+1] to pat[patlen-1] does not occur elsewhere in pat,
// the next plausible match starts at or after the mismatch.
// If, within the substring pat[pos+1 .. patlen-1], lies a prefix
// of pat, the next plausible match is here (if there are multiple
// prefixes in the substring, pick the longest). Otherwise, the
// next plausible match starts past the character aligned with 
// pat[patlen-1].
// 
// In case 2:
// pat[pos+1] to pat[patlen-1] does occur elsewhere in pat. The
// mismatch tells us that we are not looking at the end of a match.
// We may, however, be looking at the middle of a match.
// 
// The first loop, which takes care of case 1, is analogous to
// the KMP table, adapted for a 'backwards' scan order with the
// additional restriction that the substrings it considers as 
// potential prefixes are all suffixes. In the worst case scenario
// pat consists of the same letter repeated, so every suffix is
// a prefix. This loop alone is not sufficient, however:
// Suppose that pat is "ABYXCDEYX", and text is ".....ABYXCDEYX".
// We will match X, Y, and find B != E. There is no prefix of pat
// in the suffix "YX", so the first loop tells us to skip forward
// by 9 characters.
// Although superficially similar to the KMP table, the KMP table
// relies on information about the beginning of the partial match
// that the BM algorithm does not have.
//
// The second loop addresses case 2. Since suffix_length may not be
// unique, we want to take the minimum value, which will tell us
// how far away the closest potential match is.

void BoyerMoore::MakeDelta2(int *delta2, uint8_t *pat, int32_t patlen) 
{
    int p;
    int last_prefix_index = patlen-1;
 
    // first loop
    for (p=patlen-1; p>=0; p--) {
        if (IsPrefix(pat, patlen, p+1)) {
            last_prefix_index = p+1;
        }
        delta2[p] = last_prefix_index + (patlen-1 - p);
    }
 
    // second loop
    for (p=0; p < patlen-1; p++) {
        int slen = IsPrefix(pat, patlen, p);
        if (pat[p - slen] != pat[patlen-1 - slen]) {
            delta2[patlen-1 - slen] = patlen-1 - p + slen;
        }
    }
}
 
//-----------------------------------------------------------------------------
// BoyerMoore::Search (static)
//
// Executes a Boyer-Moore pattern search
//
// Arguments:
//
//	haystack		- Binary data to be searched
//	haystacklen		- Length of binary data to be searched in bytes
//	needle			- Binary pattern to match
//	needlelen		- Length of binary pattern in bytes

void* BoyerMoore::Search(void* haystack, size_t haystacklen, void* needle, size_t needlelen)
{
	// The unmodified algorithm code expects 32-bit unsigned length arguments
	if((haystacklen > UINT32_MAX) || (needlelen > UINT32_MAX)) throw Exception(E_INVALIDARG);

	// Cast the arguments and invoke the Boyer-Moore pattern search
	return reinterpret_cast<void*>(BoyerMooreSearch(reinterpret_cast<uint8_t*>(haystack), static_cast<uint32_t>(haystacklen), 
		reinterpret_cast<uint8_t*>(needle), static_cast<uint32_t>(needlelen)));
}

//-----------------------------------------------------------------------------
// BoyerMoore::SuffixLength (private, static)
//
// length of the longest suffix of word ending on word[pos].
// suffix_length("dddbcabc", 8, 4) = 2

int BoyerMoore::SuffixLength(uint8_t *word, int wordlen, int pos) 
{
    int i;
    // increment suffix length i to the first mismatch or beginning
    // of the word
    for (i = 0; (word[pos-i] == word[wordlen-1-i]) && (i < pos); i++);
    return i;
}
 
//-----------------------------------------------------------------------------

#pragma warning(pop)
