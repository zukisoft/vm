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

#ifndef __CONSOLE_H_
#define __CONSOLE_H_
#pragma once

#include <mutex>
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

	// Read
	//
	// Reads a single character from the console
	tchar_t Read(void);

	// ReadLine
	//
	// Reads a line of text from the console
	std::tstring ReadLine(void);

	// SetBufferSize
	//
	// Sets the console screen buffer size
	void SetBufferSize(int16_t width, int16_t height) const;

	// SetCursorPosition
	//
	// Sets the console cursor position
	void SetCursorPosition(int16_t left, int16_t top) const;

	// SetWindowPosition
	//
	// Sets the position of the console window relative to the screen buffer
	void SetWindowPosition(int16_t left, int16_t right) const;

	// SetWindowSize
	//
	// Sets the size of the console screen buffer window
	void SetWindowSize(int16_t width, int16_t height) const;

	// Write (fundamental types)
	//
	// Writes a value to the console
	template<typename _type> typename std::enable_if<std::is_fundamental<_type>::value, void>::type
	Write(_type value) const { Write(std::to_tstring(value)); }

	// Write (bool)
	//
	// Writes a value to the console
	void Write(bool value) const { Write(std::tstring(value ? _T("true") : _T("false"))); }

	// Write (const tchar_t*)
	//
	// Writes a value to the console
	void Write(const tchar_t* value) const { Write(std::tstring(value)); }

	// Write (tstring)
	//
	// Writes a value to the console
	void Write(const std::tstring& value) const { WriteConsole(m_stdout, value.data(), static_cast<DWORD>(value.size()), nullptr, nullptr); }

	// TODO: Write (format string)
	// TODO: Write (character iterator)

	// WriteLine (void)
	//
	// Writes a blank line to the console
	void WriteLine(void) const { WriteConsole(m_stdout, _T("\r\n"), 2, nullptr, nullptr); }

	// WriteLine (fundamental types)
	//
	// Writes a value to the console and appends a CRLF pair
	template<typename _type> typename std::enable_if<std::is_fundamental<_type>::value, void>::type
	WriteLine(const _type& value) const { WriteLine(std::to_tstring(value)); }

	// WriteLine (bool)
	//
	// Writes a value to the console and appends a CRLF pair
	void WriteLine(bool value) const { WriteLine(std::tstring(value ? _T("true") : _T("false"))); }

	// WriteLine (const tchar_t*)
	void WriteLine(const tchar_t* value) const { WriteLine(std::tstring(value)); }

	// WriteLine (tstring)
	//
	// Writes a value to the console with an appended CRLF pair
	void WriteLine(const std::tstring& value) const
	{
		// Write the string in one call by pre-appending the CRLF
		std::tstring formatted = value + _T("\r\n");
		WriteConsole(m_stdout, formatted.data(), static_cast<DWORD>(formatted.size()), nullptr, nullptr);
	}

	// TODO: WriteLine (format string)
	// TODO: WriteLine (character iterator)

	//-------------------------------------------------------------------------
	// Properties

	// BufferHeight
	//
	// Gets/Sets the console screen buffer height
	__declspec(property(get=getBufferHeight, put=putBufferHeight)) int16_t BufferHeight;
	int16_t getBufferHeight(void) const;
	void putBufferHeight(int16_t value) const;

	// BufferWidth
	//
	// Gets/Sets the console screen buffer width
	__declspec(property(get=getBufferWidth, put=putBufferWidth)) int16_t BufferWidth;
	int16_t getBufferWidth(void) const;
	void putBufferWidth(int16_t value) const;

	// CapsLock
	//
	// Gets the state of the CAPS LOCK key
	__declspec(property(get=getCapsLock)) bool CapsLock;
	bool getCapsLock(void) const;

	// CursorLeft
	//
	// Gets/Sets the console screen buffer height
	__declspec(property(get=getCursorLeft, put=putCursorLeft)) int16_t CursorLeft;
	int16_t getCursorLeft(void) const;
	void putCursorLeft(int16_t value) const;

	// CursorTop
	//
	// Gets/Sets the console screen buffer width
	__declspec(property(get=getCursorTop, put=putCursorTop)) int16_t CursorTop;
	int16_t getCursorTop(void) const;
	void putCursorTop(int16_t value) const;

	// KeyAvailable
	//
	// Gets a flag if there is an input key available for input
	__declspec(property(get=getKeyAvailable)) bool KeyAvailable;
	bool getKeyAvailable(void) const;

	// LargestWindowHeight
	//
	// Gets the height of the largest possible console window
	__declspec(property(get=getLargestWindowHeight)) int16_t LargestWindowHeight;
	int16_t getLargestWindowHeight(void) const;

	// LargestWindowWidth
	//
	// Gets the width of the largest possible console window
	__declspec(property(get=getLargestWindowWidth)) int16_t LargestWindowWidth;
	int16_t getLargestWindowWidth(void) const;

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

	// TreatControlCAsInput
	//
	// Gets/Sets the flag that allows CTRL+C to be an an input character
	__declspec(property(get=getTreatControlCAsInput, put=putTreatControlCAsInput)) bool TreatControlCAsInput;
	bool getTreatControlCAsInput(void) const;
	void putTreatControlCAsInput(bool value) const;

	// WindowHeight
	//
	// Gets/Sets the console screen buffer window height
	__declspec(property(get=getWindowHeight, put=putWindowHeight)) int16_t WindowHeight;
	int16_t getWindowHeight(void) const;
	void putWindowHeight(int16_t value) const;

	// WindowWidth
	//
	// Gets/Sets the console screen buffer window width
	__declspec(property(get=getWindowWidth, put=putWindowWidth)) int16_t WindowWidth;
	int16_t getWindowWidth(void) const;
	void putWindowWidth(int16_t value) const;

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

	// IsSpecialKey
	//
	// todo: words
	// todo: move to cpp file
	static bool IsSpecialKey(WORD vk)
	{
		// ripped off from .NET
		return (((vk >= VK_SHIFT) && (vk <= VK_MENU)) || (vk == VK_CAPITAL) ||
			(vk == VK_NUMLOCK) || (vk == VK_SCROLL));
	}

	//-------------------------------------------------------------------------
	// Member Variables

	HANDLE					m_stderr;			// STDERR stream handle
	HANDLE					m_stdin;			// STDIN stream handle
	HANDLE					m_stdout;			// STDOUT stream handle
	std::recursive_mutex	m_readlock;			// STDIN read lock object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __CONSOLE_H_
