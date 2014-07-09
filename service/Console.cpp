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
#include "Console.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// Console Constructor
//
// Arguments:
//
//	NONE

Console::Console()
{
	// Attach to the parent console if available, otherwise allocate one
	if(!AttachConsole(ATTACH_PARENT_PROCESS)) AllocConsole();

	// Get the input/output handles for the attached console
	m_stderr = GetStdHandle(STD_ERROR_HANDLE);
	m_stdin = GetStdHandle(STD_INPUT_HANDLE);
	m_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
}

//-----------------------------------------------------------------------------
// Console Constructor
//
// Arguments:
//
//	title		- Title to assign to the console window

Console::Console(const std::tstring& title) : Console()
{
	// After the default constructor, just set the console title
	SetConsoleTitle(title.c_str());
}

//-----------------------------------------------------------------------------
// Console Destructor

Console::~Console()
{
	FreeConsole();				// Detach from the console
}

//-----------------------------------------------------------------------------
// Console::Beep
//
// Emits a tone through the console
//
// Arguments:
//
//	NONE

void Console::Beep(void) const
{ 
	// 800/200 is what .NET uses by default use the same values
	Beep(800, 200); 
}

//-----------------------------------------------------------------------------
// Console::Beep
//
// Emits a tone through the console
//
// Arguments:
//
//	frequency		- Frequency of the sound, in Hertz
//	duration		- Duration of the sound, in milliseconds

void Console::Beep(int frequency, int duration) const
{ 
	// As of Windows 7, this function works again; it's emulated
	::Beep(static_cast<DWORD>(frequency), static_cast<DWORD>(duration)); 
}

//-----------------------------------------------------------------------------
// Console::getBufferHeight
//
// Gets the height of the attached console screen buffer

int16_t Console::getBufferHeight(void) const
{
	return ScreenBufferInfo(*this).dwSize.Y;
}

//-----------------------------------------------------------------------------
// Console::putBufferHeight
//
// Sets the height of the attach console screen buffer

void Console::putBufferHeight(int16_t value) const
{
	SetBufferSize(BufferWidth, value);
}

//-----------------------------------------------------------------------------
// Console::getBufferWidth
//
// Gets the width of the attached console screen buffer

int16_t Console::getBufferWidth(void) const
{
	return ScreenBufferInfo(*this).dwSize.X;
}

//-----------------------------------------------------------------------------
// Console::putBufferWidth
//
// Sets the width of the attach console screen buffer

void Console::putBufferWidth(int16_t value) const
{
	SetBufferSize(value, BufferHeight);
}

//-----------------------------------------------------------------------------
// Console::getCapsLock
//
// Gets the state of the CAPS LOCK key

bool Console::getCapsLock(void) const
{
	return ((GetKeyState(VK_CAPITAL) & 1) == 1);
}

//-----------------------------------------------------------------------------
// Console::Clear
//
// Clears the console contents
//
// Arguments:
//
//	NONE

void Console::Clear(void) const
{
	ScreenBufferInfo	info(*this);		// Screen buffer information
	DWORD				result;				// Result from fill operation

	// Fill the console with space characters using the current attibutes; move cursor to 0,0 afterwards
	if(!FillConsoleOutputCharacter(m_stdout, _T(' '), info.dwSize.X * info.dwSize.Y, { 0, 0 }, &result)) throw Win32Exception();
	if(!FillConsoleOutputAttribute(m_stdout, info.wAttributes, info.dwSize.X * info.dwSize.Y, { 0, 0 }, &result)) throw Win32Exception();
	if(!SetConsoleCursorPosition(m_stdout, { 0, 0 })) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Console::getCursorLeft
//
// Gets the X coordinate of the attached console's cursor

int16_t Console::getCursorLeft(void) const
{
	return ScreenBufferInfo(*this).dwCursorPosition.X;
}

//-----------------------------------------------------------------------------
// Console::putCursorLeft
//
// Sets the X coordinate of the attached console's cursor

void Console::putCursorLeft(int16_t value) const
{
	SetCursorPosition(value, CursorTop);
}

//-----------------------------------------------------------------------------
// Console::getCursorTop
//
// Gets the Y coordinate of the attached console's cursor

int16_t Console::getCursorTop(void) const
{
	return ScreenBufferInfo(*this).dwCursorPosition.Y;
}

//-----------------------------------------------------------------------------
// Console::putCursorTop
//
// Sets the Y coordinate of the attached console's cursor

void Console::putCursorTop(int16_t value) const
{
	SetCursorPosition(CursorLeft, value);
}

//-----------------------------------------------------------------------------
// Console::getLargestWindowHeight
//
// Gets the height of the attached console screen LargestWindow

int16_t Console::getLargestWindowHeight(void) const
{
	return GetLargestConsoleWindowSize(m_stdout).Y;
}

//-----------------------------------------------------------------------------
// Console::getLargestWindowWidth
//
// Gets the width of the attached console screen LargestWindow

int16_t Console::getLargestWindowWidth(void) const
{
	return GetLargestConsoleWindowSize(m_stdout).X;
}

//-----------------------------------------------------------------------------
// Console::getNumLock
//
// Gets the state of the NUM LOCK key

bool Console::getNumLock(void) const
{
	return ((GetKeyState(VK_NUMLOCK) & 1) == 1);
}

//-----------------------------------------------------------------------------
// Console::SetBufferSize
//
// Sets the width and height of the attached console screen buffer
//
// Arguments:
//
//	width		- New screen buffer width
//	height		- New screen buffer height

void Console::SetBufferSize(int16_t width, int16_t height) const
{
	// Check the boundaries of the specified width and height values against the console window
	SMALL_RECT window = ScreenBufferInfo(*this).srWindow;
	if((width < window.Right + 1) || (height < window.Bottom + 1)) throw Exception(E_INVALIDARG);

	// Attempt to set the attached console's screen buffer size
	if(!SetConsoleScreenBufferSize(m_stdout, { width, height })) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Console::SetCursorPosition
//
// Sets the position of the cursor in the attached console window
//
// Arguments:
//
//	left		- New cursor X coordinate
//	top			- New cursor Y coordinate

void Console::SetCursorPosition(int16_t left, int16_t top) const
{
	// Check the specified left and top coordinates against the console screen buffer
	COORD buffer = ScreenBufferInfo(*this).dwSize;
	if((left >= buffer.X) || (top >= buffer.Y)) throw Exception(E_INVALIDARG);

	// Attempt to set the attached console's cursor position
	if(!SetConsoleCursorPosition(m_stdout, { left, top })) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Console::SetWindowPosition
//
// Sets the position of the console window relative to the screen buffer
//
// Arguments:
//
//	left		- New window X coordinate
//	top			- New window Y coordinate

void Console::SetWindowPosition(int16_t left, int16_t top) const
{
	// Window position can never be negative
	if((left < 0) || (top < 0)) throw Exception(E_INVALIDARG);

	// Adjust the current window SMALL_RECT based on the new left and top
	SMALL_RECT window = ScreenBufferInfo(*this).srWindow;
	window.Bottom -= (window.Top - top);
	window.Right -= (window.Left - left);
	window.Left = left;
	window.Top = top;

	// Attempt to set the new console window position
	if(!SetConsoleWindowInfo(m_stdout, TRUE, &window)) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Console::getTitle
//
// Gets the current console title

std::tstring Console::getTitle(void) const 
{
	std::vector<tchar_t> title(_MAX_PATH);
	
	// Get the attach console's title string
	size_t length = GetConsoleTitle(title.data(), title.size());
	if(length == 0) throw Win32Exception();

	// Use either the returned length or the buffer length
	return std::tstring(title.data(), min(length, title.size()));
}

//-----------------------------------------------------------------------------
// Console::putTitle
//
// Sets the current console title

void Console::putTitle(const std::tstring& value) const
{
	SetConsoleTitle(value.c_str());
}

//-----------------------------------------------------------------------------
// Console::ScreenBufferInfo Constructor
//
// Arguments:
//
//	parent		- Reference to the parent Console instance

Console::ScreenBufferInfo::ScreenBufferInfo(const Console& parent)
{
	// Try in order: STDOUT, STDERR and STDIN
	if(GetConsoleScreenBufferInfo(parent.m_stdout, this)) return;
	else if(GetConsoleScreenBufferInfo(parent.m_stderr, this)) return;
	else if(GetConsoleScreenBufferInfo(parent.m_stdin, this)) return;
	else throw Win32Exception();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
