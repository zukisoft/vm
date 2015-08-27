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
// Console::getKeyAvailable
//
// Gets a flag if there is an input key available to be read

bool Console::getKeyAvailable(void) const
{
	//INPUT_RECORD		input;			// INPUT_RECORD from PeekConsoleInput()
	//DWORD				read;			// Number of input records read

	// TODO: Check this
	//while(true) {

	//	// Peek at the next input record; false if nothing is available
	//	if(!PeekConsoleInput(m_stdin, &input, 1, &read)) throw Win32Exception();
	//	if(read == 0) return false;
	//	
	//	// If this is a key down event that isn't for a special key, we're done
	//	if((input.EventType == KEY_EVENT) && (input.Event.KeyEvent.bKeyDown) && 
	//		!IsSpecialKey(input.Event.KeyEvent.wVirtualKeyCode)) return true;

	//	// Pop the INPUT_RECORD from the input queue and iterator again
	//	if(!ReadConsoleInput(m_stdin, &input, 1, &read)) throw Win32Exception();
	//}

	return false;
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

tchar_t Console::Read(void)
{
	std::lock_guard<std::recursive_mutex> lock(m_readlock);

	return 0;
}

//-----------------------------------------------------------------------------
// Console::ReadLine
//
// Reads a line of text from the console
// TODO: how to deal with EOF (CTRL+Z)???
//
// Arguments:
//
//	NONE

std::tstring Console::ReadLine(void)
{
	std::vector<tchar_t>	accumulator;			// Character accumulator
	DWORD					mode = 0;				// Original console mode
	DWORD					read = 0;				// Characters read from the console
	tchar_t					next;					// Next character read from console

	// Prevent multiple threads from reading the console at the same time
	std::lock_guard<std::recursive_mutex> lock(m_readlock);

	// Ensure LINE_INPUT and ECHO_INPUT are enabled for the input mode
	if(!GetConsoleMode(m_stdin, &mode)) throw Win32Exception();
	if(!SetConsoleMode(m_stdin, mode | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)) throw Win32Exception();

	try {

		// Repeatedly read characters from the console until CR has been detected
		if(!ReadConsole(m_stdin, &next, 1, &read, nullptr)) throw Win32Exception();
		while(next != _T('\n')) { 
		
			if(next != _T('\r')) accumulator.push_back(next); 
			if(!ReadConsole(m_stdin, &next, 1, &read, nullptr)) throw Win32Exception();
		}
	}

	// Be sure the restore the original console mode flags on any exception
	catch(...) { SetConsoleMode(m_stdin, mode); throw; }

	// Restore the previously set input mode flags
	SetConsoleMode(m_stdin, mode);

	// Convert the accumulated character data as a tstring instance
	return std::tstring(accumulator.data(), accumulator.size());
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
// Console::SetWindowSize
//
// Sets the size of the console screen buffer window
//
// Arguments:
//
//	width		- New window width
//	height		- New window height

void Console::SetWindowSize(int16_t width, int16_t height) const
{
	ScreenBufferInfo info(*this);			// Current screen buffer information

	if((width < 0) || (height < 0)) throw Exception(E_INVALIDARG);

	//COORD size = info.dwSize;

	// TODO
	throw Exception(E_NOTIMPL);
}

//-----------------------------------------------------------------------------
// Console::getTitle
//
// Gets the current console title

std::tstring Console::getTitle(void) const 
{
	std::vector<tchar_t> title(_MAX_PATH);
	
	// Get the attach console's title string
	size_t length = GetConsoleTitle(title.data(), static_cast<DWORD>(title.size()));
	if(length == 0) throw Win32Exception();

	// Use either the returned length or the buffer length
	return std::tstring(title.data(), std::min(length, title.size()));
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
// Console::getTreatControlCAsInput
//
// Gets the flag indicating that CTRL+C should be considered normal input

bool Console::getTreatControlCAsInput(void) const
{
	DWORD mode = 0;
	if(!GetConsoleMode(m_stdin, &mode)) throw Win32Exception();

	return ((mode & ENABLE_PROCESSED_INPUT) == ENABLE_PROCESSED_INPUT);
}

//-----------------------------------------------------------------------------
// Console::putTreatControlCAsInput
//
// Sets the flag indicating that CTRL+C should be considered normal input

void Console::putTreatControlCAsInput(bool value) const
{
	DWORD mode = 0;
	if(!GetConsoleMode(m_stdin, &mode)) throw Win32Exception();

	// Set or clear ENABLED_PROCESSED_INPUT from the bitmask returned
	if(!SetConsoleMode(m_stdin, (value) ? mode | ENABLE_PROCESSED_INPUT : 
		mode & ~ENABLE_PROCESSED_INPUT)) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Console::getWindowHeight
//
// Gets the height of the attached console screen buffer window

int16_t Console::getWindowHeight(void) const
{
	ScreenBufferInfo info(*this);
	return info.srWindow.Bottom - info.srWindow.Top + 1;
}

//-----------------------------------------------------------------------------
// Console::putWindowHeight
//
// Sets the height of the attach console screen buffer window

void Console::putWindowHeight(int16_t value) const
{
	SetWindowSize(WindowWidth, value);
}

//-----------------------------------------------------------------------------
// Console::getWindowWidth
//
// Gets the width of the attached console screen buffer window

int16_t Console::getWindowWidth(void) const
{
	ScreenBufferInfo info(*this);
	return info.srWindow.Right - info.srWindow.Left + 1;
}

//-----------------------------------------------------------------------------
// Console::putWindowWidth
//
// Sets the width of the attach console screen buffer window

void Console::putWindowWidth(int16_t value) const
{
	SetWindowSize(value, WindowHeight);
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
