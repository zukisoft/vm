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
#include "PathSplitter.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------'
// PathSplitter Constructor
//
// Arguments:
//
//	path		- Pointer to the path to be parsed

PathSplitter::PathSplitter(const tchar_t* path) : m_path((path) ? _tcslen(path) + 1 : 1, 0)
{
	// Copy the string data from the original pointer into the vector<>
	if(path) memcpy(m_path.data(), path, m_path.size() * sizeof(tchar_t));

	// Determine if the path is absolute by checking for a leading slash character
	m_absolute = (m_path[0] == _T('/'));

	// Work backwards from the end of the string to find the last slash and
	// change it into the null terminator for the branch portion
	tchar_t* end = m_path.data() + (m_path.size() - 1);
	while((end >= m_path.data()) && (*end != _T('/'))) --end;
	if(end >= m_path.data()) *end = 0;

	// The leaf string starts one character beyond the calculated end pointer
	m_leaf = (end + 1);
	
	// If there is a branch, it starts at the original head of the string, otherwise
	// set it to the null terminator at the very end of the buffer
	m_branch = (end > m_path.data()) ? m_path.data() : &m_path[m_path.size() - 1];

	// Move beyond any leading slash characters in the branch path to make it relative,
	// the caller can check the .Absolute property to determine if it was present
	while(*m_branch == _T('/')) ++m_branch;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
