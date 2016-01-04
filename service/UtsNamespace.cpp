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

#include "stdafx.h"
#include "UtsNamespace.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// UtsNamespace Constructor (private)
//
// Arguments:
//
//	hostname	- Initial hostname string to assign
//	domainname	- Initial domain name string to assign

UtsNamespace::UtsNamespace(const std::string& hostname, const std::string& domainname) : 
	m_hostname(hostname), m_domainname(domainname)
{
}

//-----------------------------------------------------------------------------
// UtsNamespace::Create (static)
//
// Constructs a new UtsNamespace instance
//
// Arguments:
//
//	NONE

std::shared_ptr<UtsNamespace> UtsNamespace::Create(void)
{
	// Construct a new UtsNamespace with blank host and domain name strings
	return std::make_shared<UtsNamespace>();
}

//-----------------------------------------------------------------------------
// UtsNamespace::Create (static)
//
// Constructs a new UtsNamespace instance
//
// Arguments:
//
//	utsns	- Existing UtsNamespace instance to duplicate

std::shared_ptr<UtsNamespace> UtsNamespace::Create(const std::shared_ptr<UtsNamespace>& utsns)
{
	// Construct a new UtsNamespace with copies of the existing host and domain name strings
	return std::make_shared<UtsNamespace>(utsns->m_hostname, utsns->m_domainname);
}

//-----------------------------------------------------------------------------
// UtsNamespace::getDomainName
//
// Gets the contained domain name string

std::string UtsNamespace::getDomainName(void)
{
	sync::critical_section::scoped_lock critsec(m_lock);
	return m_domainname;
}

//-----------------------------------------------------------------------------
// UtsNamespace::putDomainName
//
// Sets the contained domain name string

void UtsNamespace::putDomainName(const std::string& value)
{
	sync::critical_section::scoped_lock critsec(m_lock);
	m_domainname = value;
}

//-----------------------------------------------------------------------------
// UtsNamespace::putDomainName
//
// Sets the contained domain name string

void UtsNamespace::putDomainName(const char_t* value)
{
	sync::critical_section::scoped_lock critsec(m_lock);
	m_domainname = value;
}

//-----------------------------------------------------------------------------
// UtsNamespace::getHostName
//
// Gets the contained host name string

std::string UtsNamespace::getHostName(void)
{
	sync::critical_section::scoped_lock critsec(m_lock);
	return m_hostname;
}

//-----------------------------------------------------------------------------
// UtsNamespace::putHostName
//
// Sets the contained host name string

void UtsNamespace::putHostName(const std::string& value)
{
	sync::critical_section::scoped_lock critsec(m_lock);
	m_hostname = value;
}

//-----------------------------------------------------------------------------
// UtsNamespace::putHostName
//
// Sets the contained host name string

void UtsNamespace::putHostName(const char_t* value)
{
	sync::critical_section::scoped_lock critsec(m_lock);
	m_hostname = value;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
