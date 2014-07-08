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

#ifndef __CONSOLE_H_
#define __CONSOLE_H_
#pragma once

#include "Win32Exception.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// Console
//
// Provides a System::Console-like class for manipulating the basic windows console.
// Many interesting methods and properties have been left out, this is currently
// only intended as a quick way to get a console from a Windows subsystem executable

class Console
{
public:

	// Constructors
	Console();
	explicit Console(const std::tstring& title);

	// Destructor
	~Console();

	//-------------------------------------------------------------------------
	// Member Functions

	// Beep
	//
	// Plays a beep through the console
	void Beep(void) const;
	void Beep(int frequency, int duration) const;

	// Clear
	//
	// Clears the console contents
	void Clear(void) const;

	// SetBufferSize
	//
	// Sets the console screen buffer size
	void SetBufferSize(size_t width, size_t height) const;

	// SetCursorPosition
	//
	// Sets the console cursor position
	void SetCursorPosition(size_t left, size_t top) const;

	template <typename ... _remaining>
	void WriteLine(const _remaining&... remaining)
	{
		std::tstring value;
		WriteLine(value, remaining...);
	}

	//-------------------------------------------------------------------------
	// Properties

	// BufferHeight
	//
	// Gets/Sets the console screen buffer height
	__declspec(property(get=getBufferHeight, put=putBufferHeight)) size_t BufferHeight;
	size_t getBufferHeight(void) const;
	void putBufferHeight(size_t value) const;

	// BufferWidth
	//
	// Gets/Sets the console screen buffer width
	__declspec(property(get=getBufferWidth, put=putBufferWidth)) size_t BufferWidth;
	size_t getBufferWidth(void) const;
	void putBufferWidth(size_t value) const;

	// CapsLock
	//
	// Gets the state of the CAPS LOCK key
	__declspec(property(get=getCapsLock)) bool CapsLock;
	bool getCapsLock(void) const;

	// CursorLeft
	//
	// Gets/Sets the console screen buffer height
	__declspec(property(get=getCursorLeft, put=putCursorLeft)) size_t CursorLeft;
	size_t getCursorLeft(void) const;
	void putCursorLeft(size_t value) const;

	// CursorTop
	//
	// Gets/Sets the console screen buffer width
	__declspec(property(get=getCursorTop, put=putCursorTop)) size_t CursorTop;
	size_t getCursorTop(void) const;
	void putCursorTop(size_t value) const;

	// LargestWindowHeight
	//
	// Gets the height of the largest possible console window
	__declspec(property(get=getLargestWindowHeight)) size_t LargestWindowHeight;
	size_t getLargestWindowHeight(void) const;

	// LargestWindowWidth
	//
	// Gets the width of the largest possible console window
	__declspec(property(get=getLargestWindowWidth)) size_t LargestWindowWidth;
	size_t getLargestWindowWidth(void) const;

	// NumLock
	//
	// Gets the state of the NUM LOCK key
	__declspec(property(get=getNumLock)) bool NumLock;
	bool getNumLock(void) const;

	// Title
	//
	// Gets/Sets the console title string
	__declspec(property(get=getTitle, put=putTitle)) std::tstring Title;
	std::tstring getTitle(void) const;
	void putTitle(const std::tstring& value) const;

private:

	Console(const Console&)=delete;
	Console& operator=(const Console&)=delete;

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// ScreenBufferInfo
	//
	// Automatically loads itself by calling GetConsoleScreenBufferInfo()
	struct ScreenBufferInfo : CONSOLE_SCREEN_BUFFER_INFO
	{
		explicit ScreenBufferInfo(const Console& parent);
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	template <typename _next, typename... _remaining>
	void WriteLine(std::tstring& value, const _next& next, const _remaining&... remaining)
	{
		// append next to string
		WriteLine(value, remaining...);
	}

	void WriteLine(std::tstring& value)
	{
		value.append(_T("WriteLine!!\r\n"));
		WriteConsole(m_stdout, value.data(), value.size(), nullptr, nullptr);
	}

	//-------------------------------------------------------------------------
	// Member Variables

	HANDLE				m_stderr;				// STDERR stream handle
	HANDLE				m_stdin;				// STDIN stream handle
	HANDLE				m_stdout;				// STDOUT stream handle
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __CONSOLE_H_
