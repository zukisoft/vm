//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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

#ifndef __CAPABILITY_H_
#define __CAPABILITY_H_
#pragma once

#include <stdint.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Capability
//
// TODO FUTURE: USE THREAD LOCAL STORAGE OR SOMETHING ON THE RPC THREAD TO
// DICTATE WHAT THE CAPABILITIES ARE; INITIAL VERSION WILL JUST GRANT ANYTHING

class Capability final
{
public:

	// Copy Constructor
	//
	Capability(const Capability&)=default;

	// Destructor
	//
	~Capability()=default;

	// bitwise or operator
	//
	Capability operator|(const Capability rhs) const;

	//-------------------------------------------------------------------------
	// Fields

	// todo: comments, put in alphabetical order

	static const Capability	None;
	static const Capability	ChangeFileOwnership;
	static const Capability	BypassFilePermissions;
	static const Capability	BypassFileReadPermissions;
	static const Capability	BypassFileRestrictions;
	static const Capability	BypassEffectiveFileRestrictions;
	static const Capability	BypassSignalPermissions;
	static const Capability	ChangeProcessGroup;
	static const Capability	ChangeProcessUser;
	static const Capability	ModifyCapabilities;
	static const Capability	SetImmutableFlags;
	static const Capability	BindPrivilegedPorts;
	static const Capability	MakeSocketBroadcasts;
	static const Capability	NetworkAdmin;
	static const Capability	UseRawSockets;
	static const Capability	LockMemory;
	static const Capability	BypassSystemVPermissions;
	static const Capability	LoadModules;
	static const Capability	RawInputOutput;
	static const Capability	ChangeRootDirectory;
	static const Capability	TraceProcesses;
	static const Capability	EnableProcessAccounting;
	static const Capability	SystemAdmin;
	static const Capability	Reboot;
	static const Capability	SetProcessPriorities;
	static const Capability	OverrideLimits;
	static const Capability	SetClocks;
	static const Capability	ConfigureTerminals;
	static const Capability	CreateSpecialFiles;
	static const Capability	EstablishFileLeases;
	static const Capability	WriteAuditLog;
	static const Capability	AuditControl;
	static const Capability	SetFileCapabilities;
	static const Capability	ConfigureMandatoryAccess;
	static const Capability	BypassMandatoryAccess;
	static const Capability	ConfigureSystemLog;
	static const Capability	TriggerWakes;
	static const Capability	BlockSuspend;
	static const Capability	ReadAuditLog;

	//-------------------------------------------------------------------------
	// Member Functions

	// Check
	//
	// Checks the specified capabilities
	static bool Check(const Capability& capability);

	// Demand
	//
	// Demands the specified capabilities
	static void Demand(const Capability& capability);

private:

	Capability& operator=(const Capability&)=delete;

	// Instance Constructor
	//
	Capability(uint64_t mask);

	//-------------------------------------------------------------------------
	// Member Variables

	const uint64_t			m_mask;			// Capabilities mask
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __CAPABILITY_H_
