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
#include "PathIterator.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// PathIterator Constructor
//
// Arguments:
//
//	path		- Pointer to the path to be iterated

PathIterator::PathIterator(const uapi::char_t* path) : m_path((path) ? strlen(path) + 1 : 1, 0)
{
	// Skip path any leading slashes in the original path, not concerned with them
	while((path) && (*path == '/')) ++path;

	// Copy the string data from the original pointer into the vector<>
	if(path) memcpy(m_path.data(), path, m_path.size() * sizeof(uapi::char_t));

	m_consumed = &m_path[m_path.size() - 1];		// Nothing consumed yet

	// Break the current component out from the data and move the remaining
	// pointer to just beyond that break
	m_current = m_remaining = m_path.data();
	while((*m_remaining) && (*m_remaining != _T('/'))) ++m_remaining;
	if(*m_remaining) *m_remaining++ = 0;
}

//-----------------------------------------------------------------------------
// PathIterator::operator++

PathIterator& PathIterator::operator++()
{
	// Reset the consumed pointer back to the beginning, it may be positioned
	// at the null terminator at the end of the buffer
	m_consumed = m_path.data();

	// Replace the slash character before the current component and advance it
	if((m_current > m_path.data()) && (m_current < m_remaining)) (*(m_current - 1) = _T('/'));
	m_current = m_remaining;

	// Advance the remaining pointer and replace the slash with a null terminator
	while((*m_remaining) && (*m_remaining != _T('/'))) ++m_remaining;
	if(*m_remaining) *m_remaining++ = 0;

	return *this;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
