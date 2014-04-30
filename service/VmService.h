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

#ifndef __VMSERVICE_H_
#define __VMSERVICE_H_
#pragma once

#include "resource.h"

#pragma warning(push, 4)			

//---------------------------------------------------------------------------
// Class VmService

class VmService : public SVCTL::Service<VmService>
{
public:

	VmService();

	//-----------------------------------------------------------------------
	// SVCTL Declarations

	DECLARE_SERVICE_NAME_ID(IDS_VMSERVICE_NAME)
	DECLARE_SERVICE_DISPLAYNAME_ID(IDS_VMSERVICE_DISPLAYNAME)
	DECLARE_SERVICE_DESCRIPTION_ID(IDS_VMSERVICE_DESCRIPTION)
	DECLARE_SERVICE_TYPE(SERVICE_WIN32_OWN_PROCESS)
	DECLARE_SERVICE_START_TYPE(SERVICE_DEMAND_START)

	//-----------------------------------------------------------------------
	// SVCTL Control Map

	BEGIN_CONTROL_MAP(VmService)
		HANDLER_EVENT(SERVICE_CONTROL_STOP, m_hevtStop)
	END_CONTROL_MAP()

	//-----------------------------------------------------------------------
	// SVCTL Parameter Map

	//BEGIN_PARAMETER_MAP()
	//	DWORD_PARAMETER_ID(IDP_DISKACTIVITYSERVICE_INTERVAL, 2500)
	//	BOOLEAN_PARAMETER_ID(IDP_DISKACTIVITYSERVICE_FILESIZE, 8)
	//END_PARAMETER_MAP()

private:

	VmService(const VmService &rhs)=delete;
	VmService& operator=(const VmService &rhs)=delete;

	//-----------------------------------------------------------------------
	// Private Member Functions

	// Init (Service)
	//
	// Performs service initializations
	DWORD Init(DWORD dwArgc, LPTSTR *rgszArgv);

	// Run (Service)
	//
	// Executes the main service loop
	DWORD Run(void);

	// Term (Service)
	//
	// Performs service shutdown
	void Term(void);

	//-----------------------------------------------------------------------
	// Member Variables

	HANDLE			m_hevtStop;			// Service STOP kernel event object
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMSERVICE_H_
