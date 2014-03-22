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

#include "stdafx.h"					// Include project pre-compiled headers
#include "VmService.h"				// Include VmService class declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

//---------------------------------------------------------------------------
// VmService Constructor
//
// Arguments:
//
//	NONE

VmService::VmService() : m_hevtStop(NULL)
{
}

//---------------------------------------------------------------------------
// VmService::Init (private)
//
// Initializes the service class object, in preparation for service start
//
// Arguments :
//
//	dwArgc		- Number of service command line arguments
//	rgszArgv	- Array of service command line argument strings	

DWORD VmService::Init(DWORD dwArgc, LPTSTR *rgszArgv)
{
	DWORD						dwResult;						// Result from function call

	UNREFERENCED_PARAMETER(dwArgc);
	UNREFERENCED_PARAMETER(rgszArgv);

	// Attempt to create the service STOP kernel event object
	m_hevtStop = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(!m_hevtStop) { 
		
		dwResult = GetLastError();
		//LogEvent(SVCTL::Win32Event(E_DISKACTIVITY_CREATESTOPEVENT, dwResult));
		Term();
		return dwResult;
	}

	return ERROR_SUCCESS;			// Service successfully initialized
}

//---------------------------------------------------------------------------
// VmService::Run (private)
//
// Entry point for the main service thread
//
// Arguments :
//
//	NONE

DWORD VmService::Run(void)
{
	// For now, all we need is a simple idle loop waiting for the STOP event
	WaitForSingleObject(m_hevtStop, INFINITE);

	return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
// VmService::Term (private)
//
// Uninitializes the service class object, after the service has been stopped
//
// Arguments :
//
//	NONE

void VmService::Term(void)
{
	// Close and reset the STOP event object
	if(m_hevtStop) CloseHandle(m_hevtStop);
	m_hevtStop = NULL;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
