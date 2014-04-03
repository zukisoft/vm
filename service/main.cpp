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
#include "resource.h"				// Include project resource declarations
#include "VmService.h"				// Include VmService declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings
#pragma warning(disable:4100)		// "unreferenced formal parameter"

//---------------------------------------------------------------------------
// SVCTL Service Map

BEGIN_SVCTL_MAP(g_pServiceMap)
	SVCTL_MAP_ENTRY(VmService)
END_SVCTL_MAP()

//---------------------------------------------------------------------------
// InitError
//
// Displays a fatal process initialization error to the user.  Not very
// informative, this is more for debugging startup than anything else
//
// Arguments :
//
//	pwszDescription		- Error description to be displayed
//	dwError				- Error code to be displayed

int InitError(LPCWSTR pwszDescription, DWORD dwError)
{
	bool			bShowHex;			// Flag to display in hexadecimal
	LPWSTR			pwszMessage;		// Buffer for the error message
	SVCTL::String	appName;			// Application name string

	appName.LoadResource(IDS_APPLICATION_DISPLAYNAME);

	// If the high bit is set on dwError, display as a COM error message
	
	bShowHex = ((dwError & 0x80000000) == 0x80000000);

	pwszMessage = new WCHAR[2048];						// Allocate 2K buffer
	if(!pwszMessage) return static_cast<int>(dwError);	// Insufficient memory

	// Format and display the pathetic error message to be user (good thing these
	// errors never actually occur huh?)

	_snwprintf_s(pwszMessage, 1023, _TRUNCATE, (bShowHex) ? L"%s : 0x%08X" : L"%s : %d", pwszDescription, dwError);
	MessageBox(NULL, pwszMessage, appName, MB_OK | MB_ICONHAND | MB_SERVICE_NOTIFICATION);

	delete[] pwszMessage;					// Release the message buffer

	return static_cast<int>(dwError);
}

//---------------------------------------------------------------------------
// wWinMain
//
// Main application entry point
//
// Arguments :
//
//	hInstance			- Application instance handle
//	hPrevInstance		- Unused in Win32, always set to NULL
//	pwszCommandLine		- Pointer to application command line string
//	nCmdShow			- Application initial display flags

#pragma comment(linker, "/entry:wWinMainCRTStartup")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
					LPWSTR pwszCommandLine, int nCmdShow)
{
	SVCTL::ServiceManager	svcManager;			// SVCTL service manager class
	SVCTL::ComInit			comInit;			// COM initialization wrapper
	int						nArgc;				// Number of command line arguments
	LPWSTR*					rgwszArgv;			// Array of command line arguments
	int						nIndex;				// Loop index variable
	LPWSTR					pwszArg;			// Pointer to a single argument
	SVCTL::String			serviceName;		// Specific service to dispatch
	bool					bInstall = false;	// Flag to install the service
	bool					bRemove = false;	// Flag to remove the service
	bool					bDispatch = false;	// Flag to dispatch the service
	DWORD					dwResult;			// Result from function call
//	HRESULT					hResult;			// Result from function call

//////////////////

	RPC_STATUS rpcresult = RpcServerUseAllProtseqsIf(RPC_C_PROTSEQ_MAX_REQS_DEFAULT, RemoteSystemCalls_v1_0_s_ifspec, nullptr);
	if(rpcresult != RPC_S_OK) {
	}

///////////////////

#ifdef _DEBUG

	int nDbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);	// Get current flags
	nDbgFlags |= _CRTDBG_LEAK_CHECK_DF;						// Enable leak-check
	_CrtSetDbgFlag(nDbgFlags);								// Set the new flags

#endif	// _DEBUG

	// Initialize the SVCTL service manager class object
	dwResult = svcManager.Init(g_pServiceMap);
	if(dwResult != ERROR_SUCCESS) return InitError(L"svcManager.Init()", dwResult);
	
	// Initialize the application's main thread in the process MTA

	//hResult = comInit.Initialize(COINIT_MULTITHREADED);
	//if(FAILED(hResult)) return InitError(L"comInit.Initialize()", hResult);

	//// Attempt to initialize the default proxy security for this process

	//hResult = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
	//	RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	//if(FAILED(hResult)) return InitError(L"CoInitializeSecurity()", hResult);

	// PROCESS COMMAND LINE -----------------------------------------

	rgwszArgv = CommandLineToArgvW(GetCommandLine(), &nArgc);
	if(rgwszArgv) {

		for(nIndex = 1; nIndex < nArgc; nIndex++) {

			pwszArg = rgwszArgv[nIndex];	// Cast a pointer to the argument

			if((pwszArg[0] == L'-') || (pwszArg[0] == L'/')) {

				pwszArg++;					// Move past the switch character

				// -q : set SVCTL library for quiet mode

				if(_wcsicmp(pwszArg, L"q") == 0) 
					SVCTL::SetDisplayMode(SVCTL_DISPLAY_QUIET);

				// -s : set SVCTL library for silent mode

				else if(_wcsicmp(pwszArg, L"s") == 0)
					SVCTL::SetDisplayMode(SVCTL_DISPLAY_SILENT);

				// -install
				// -regserver : Set the installation flag to true

				else if(_wcsicmp(pwszArg, L"install") == 0) bInstall = true;
				else if(_wcsicmp(pwszArg, L"regserver") == 0) bInstall = true;

				// -uninstall
				// -remove
				// -unregserver : Set the uninstallation flag to true

				else if(_wcsicmp(pwszArg, L"uninstall") == 0) bRemove = true;
				else if(_wcsicmp(pwszArg, L"remove") == 0) bRemove = true;
				else if(_wcsicmp(pwszArg, L"unregserver") == 0) bRemove = true;

				// -service[:name] : Dispatch the service(s) to the SCM.  A specific
				// service name indicates an OWN_PROCESS service, a lack of one indicates
				// that we dispatch ALL of the SHARE_PROCESS services at the same time

				else if(_wcsnicmp(pwszArg, L"service", 7) == 0) {

					if((wcslen(pwszArg) > 8) && (pwszArg[7] == ':')) serviceName = &pwszArg[8];
					bDispatch = true;
				}
			}
		}

		GlobalFree(rgwszArgv);				// Release command line arguments
	}

	// The service cannot be dispatched along with install/remove requests
	
	if(bInstall || bRemove) bDispatch = false;

	// INSTALL ------------------------------------------------------

	if(bInstall) {

		SVCTL::ActivateUserInterface(IDS_APPLICATION_DISPLAYNAME);
		svcManager.Install();
		SVCTL::DeActivateUserInterface();
	}

	// UNINSTALL ----------------------------------------------------

	if(bRemove) {

		SVCTL::ActivateUserInterface(IDS_APPLICATION_DISPLAYNAME);
		svcManager.Remove();
		SVCTL::DeActivateUserInterface();
	}

	// DISPATCH -----------------------------------------------------

	OutputDebugString(serviceName);
	if(bDispatch) svcManager.Dispatch(serviceName.IsNull() ? NULL : serviceName);

	svcManager.Term();				// Terminate the SVCTL service manager

	return 0;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
