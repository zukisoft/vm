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

#ifndef __CAPABILITIES_H_
#define __CAPABILITIES_H_
#pragma once

#include <stdint.h>
#include <linux/capability.h>

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Enum Capability
//
// Defines the specific capability flags. These are treated as a bitmask based
// on the actual Linux CAP_ definitions, note there are more than 32 of them
// so a 64-bit enum class must be used
enum class Capability : uint64_t
{
	None							= 0,
	ChangeFileOwnership				= (1ui64 << LINUX_CAP_CHOWN),
	BypassFilePermissions			= (1ui64 << LINUX_CAP_DAC_OVERRIDE),
	BypassFileReadPermissions		= (1ui64 << LINUX_CAP_DAC_READ_SEARCH),
	BypassFileRestrictions			= (1ui64 << LINUX_CAP_FOWNER),
	BypassEffectiveFileRestrictions	= (1ui64 << LINUX_CAP_FSETID),
	BypassSignalPermissions			= (1ui64 << LINUX_CAP_KILL),
	ChangeProcessGroup				= (1ui64 << LINUX_CAP_SETGID),
	ChangeProcessUser				= (1ui64 << LINUX_CAP_SETUID),
	ModifyCapabilities				= (1ui64 << LINUX_CAP_SETPCAP),
	SetImmutableFlags				= (1ui64 << LINUX_CAP_LINUX_IMMUTABLE),
	BindPrivilegedPorts				= (1ui64 << LINUX_CAP_NET_BIND_SERVICE),
	MakeSocketBroadcasts			= (1ui64 << LINUX_CAP_NET_BROADCAST),
	NetworkAdmin					= (1ui64 << LINUX_CAP_NET_ADMIN),
	UseRawSockets					= (1ui64 << LINUX_CAP_NET_RAW),
	LockMemory						= (1ui64 << LINUX_CAP_IPC_LOCK),
	BypassSystemVPermissions		= (1ui64 << LINUX_CAP_IPC_OWNER),
	LoadModules						= (1ui64 << LINUX_CAP_SYS_MODULE),
	RawInputOutput					= (1ui64 << LINUX_CAP_SYS_RAWIO),
	ChangeRootDirectory				= (1ui64 << LINUX_CAP_SYS_CHROOT),
	TraceProcesses					= (1ui64 << LINUX_CAP_SYS_PTRACE),
	EnableProcessAccounting			= (1ui64 << LINUX_CAP_SYS_PACCT),
	SystemAdmin						= (1ui64 << LINUX_CAP_SYS_ADMIN),
	Reboot							= (1ui64 << LINUX_CAP_SYS_BOOT),
	SetProcessPriorities			= (1ui64 << LINUX_CAP_SYS_NICE),
	OverrideLimits					= (1ui64 << LINUX_CAP_SYS_RESOURCE),
	SetClocks						= (1ui64 << LINUX_CAP_SYS_TIME),
	ConfigureTerminals				= (1ui64 << LINUX_CAP_SYS_TTY_CONFIG),
	CreateSpecialFiles				= (1ui64 << LINUX_CAP_MKNOD),
	EstablishFileLeases				= (1ui64 << LINUX_CAP_LEASE),
	WriteAuditLog					= (1ui64 << LINUX_CAP_AUDIT_WRITE),
	AuditControl					= (1ui64 << LINUX_CAP_AUDIT_CONTROL),
	SetFileCapabilities				= (1ui64 << LINUX_CAP_SETFCAP),
	ConfigureMandatoryAccess		= (1ui64 << LINUX_CAP_MAC_OVERRIDE),
	BypassMandatoryAccess			= (1ui64 << LINUX_CAP_MAC_ADMIN),
	ConfigureSystemLog				= (1ui64 << LINUX_CAP_SYSLOG),
	TriggerWakes					= (1ui64 << LINUX_CAP_WAKE_ALARM),
	BlockSuspend					= (1ui64 << LINUX_CAP_BLOCK_SUSPEND),
	ReadAuditLog					= (1ui64 << LINUX_CAP_AUDIT_READ),
};

//-----------------------------------------------------------------------------
// Capabilities
//
// TODO FUTURE: USE THREAD LOCAL STORAGE OR SOMETHING ON THE RPC THREAD TO
// DICTATE WHAT THE CAPABILITIES ARE; INITIAL VERSION WILL JUST GRANT ANYTHING

class Capabilities
{
public:
	//-------------------------------------------------------------------------
	// Member Functions

	// Demand
	//
	// Demands the provided capabilities on the calling thread
	static void Demand(const Capability& cap);

private:

	Capabilities()=delete;
	~Capabilities()=delete;
	Capabilities(const Capabilities&)=delete;
	Capabilities& operator=(const Capabilities&)=delete;
};

// ::Capability Bitwise Operators
inline Capability operator~(Capability lhs) {
	return static_cast<Capability>(~static_cast<uint32_t>(lhs));
}

inline Capability operator&(Capability lhs, Capability rhs) {
	return static_cast<Capability>(static_cast<uint32_t>(lhs) & (static_cast<uint32_t>(rhs)));
}

inline Capability operator|(Capability lhs, Capability rhs) {
	return static_cast<Capability>(static_cast<uint32_t>(lhs) | (static_cast<uint32_t>(rhs)));
}

inline Capability operator^(Capability lhs, Capability rhs) {
	return static_cast<Capability>(static_cast<uint32_t>(lhs) ^ (static_cast<uint32_t>(rhs)));
}

// ::Capability Compound Assignment Operators
inline Capability& operator&=(Capability& lhs, Capability rhs) 
{
	lhs = lhs & rhs;
	return lhs;
}

inline Capability& operator|=(Capability& lhs, Capability rhs) 
{
	lhs = lhs | rhs;
	return lhs;
}

inline Capability& operator^=(Capability& lhs, Capability rhs) 
{
	lhs = lhs ^ rhs;
	return lhs;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __CAPABILITIES_H_
