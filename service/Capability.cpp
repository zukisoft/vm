//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"}; to deal
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
#include "Capability.h"

#include "LinuxException.h"

#pragma warning(push, 4)

// todo: comments and alphabetical order

const Capability Capability::ChangeFileOwnership{ 1ui64 << LINUX_CAP_CHOWN };
const Capability Capability::BypassFilePermissions{ 1ui64 << LINUX_CAP_DAC_OVERRIDE };
const Capability Capability::BypassFileReadPermissions{ 1ui64 << LINUX_CAP_DAC_READ_SEARCH };
const Capability Capability::BypassFileRestrictions{ 1ui64 << LINUX_CAP_FOWNER };
const Capability Capability::BypassEffectiveFileRestrictions{ 1ui64 << LINUX_CAP_FSETID };
const Capability Capability::BypassSignalPermissions{ 1ui64 << LINUX_CAP_KILL };
const Capability Capability::ChangeProcessGroup{ 1ui64 << LINUX_CAP_SETGID };
const Capability Capability::ChangeProcessUser{ 1ui64 << LINUX_CAP_SETUID };
const Capability Capability::ModifyCapabilities{ 1ui64 << LINUX_CAP_SETPCAP };
const Capability Capability::SetImmutableFlags{ 1ui64 << LINUX_CAP_LINUX_IMMUTABLE };
const Capability Capability::BindPrivilegedPorts{ 1ui64 << LINUX_CAP_NET_BIND_SERVICE };
const Capability Capability::MakeSocketBroadcasts{ 1ui64 << LINUX_CAP_NET_BROADCAST };
const Capability Capability::NetworkAdmin{ 1ui64 << LINUX_CAP_NET_ADMIN };
const Capability Capability::UseRawSockets{ 1ui64 << LINUX_CAP_NET_RAW };
const Capability Capability::LockMemory{ 1ui64 << LINUX_CAP_IPC_LOCK };
const Capability Capability::BypassSystemVPermissions{ 1ui64 << LINUX_CAP_IPC_OWNER };
const Capability Capability::LoadModules{ 1ui64 << LINUX_CAP_SYS_MODULE };
const Capability Capability::RawInputOutput{ 1ui64 << LINUX_CAP_SYS_RAWIO };
const Capability Capability::ChangeRootDirectory{ 1ui64 << LINUX_CAP_SYS_CHROOT };
const Capability Capability::TraceProcesses{ 1ui64 << LINUX_CAP_SYS_PTRACE };
const Capability Capability::EnableProcessAccounting{ 1ui64 << LINUX_CAP_SYS_PACCT };

// Capability::SystemAdmin (static)
//
const Capability Capability::SystemAdmin{ 1ui64 << LINUX_CAP_SYS_ADMIN };

const Capability Capability::Reboot{ 1ui64 << LINUX_CAP_SYS_BOOT };
const Capability Capability::SetProcessPriorities{ 1ui64 << LINUX_CAP_SYS_NICE };
const Capability Capability::OverrideLimits{ 1ui64 << LINUX_CAP_SYS_RESOURCE };
const Capability Capability::SetClocks{ 1ui64 << LINUX_CAP_SYS_TIME };
const Capability Capability::ConfigureTerminals{ 1ui64 << LINUX_CAP_SYS_TTY_CONFIG };
const Capability Capability::CreateSpecialFiles{ 1ui64 << LINUX_CAP_MKNOD };
const Capability Capability::EstablishFileLeases{ 1ui64 << LINUX_CAP_LEASE };
const Capability Capability::WriteAuditLog{ 1ui64 << LINUX_CAP_AUDIT_WRITE };
const Capability Capability::AuditControl{ 1ui64 << LINUX_CAP_AUDIT_CONTROL };
const Capability Capability::SetFileCapabilities{ 1ui64 << LINUX_CAP_SETFCAP };
const Capability Capability::ConfigureMandatoryAccess{ 1ui64 << LINUX_CAP_MAC_OVERRIDE };
const Capability Capability::BypassMandatoryAccess{ 1ui64 << LINUX_CAP_MAC_ADMIN };
const Capability Capability::ConfigureSystemLog{ 1ui64 << LINUX_CAP_SYSLOG };
const Capability Capability::TriggerWakes{ 1ui64 << LINUX_CAP_WAKE_ALARM };
const Capability Capability::BlockSuspend{ 1ui64 << LINUX_CAP_BLOCK_SUSPEND };
const Capability Capability::ReadAuditLog{ 1ui64 << LINUX_CAP_AUDIT_READ };

//-----------------------------------------------------------------------------
// Capability Constructor (private)
//
// Arguments:
//
//	mask		- Capability mask to be assigned to this instance

Capability::Capability(uint64_t mask) : m_mask{ mask }
{
}

//-----------------------------------------------------------------------------
// Capability bitwise or operator

Capability Capability::operator|(const Capability rhs) const
{
	return Capability{ m_mask | rhs.m_mask };
}

//-----------------------------------------------------------------------------
// Capability::Check (static)
//
// Checks the specified capabilities
//
// Arguments:
//
//	capability	- Requested capabilities

bool Capability::Check(const Capability& capability)
{
	// todo
	(capability);

	return true;
}

//-----------------------------------------------------------------------------
// Capability::Demand (static)
//
// Demands the specified capabilities
//
// Arguments:
//
//	capability	- Requested capabilities

void Capability::Demand(const Capability& capability)
{
	// This is the same operation as Check(); it just throws an exception
	if(!Check(capability)) throw LinuxException{ LINUX_EPERM };
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
