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

#ifndef __UTSNAMESPACE_H_
#define __UTSNAMESPACE_H_
#pragma once

#include <memory>
#include <string>

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// UtsNamespace
//
// Provides isolation of host and domain name identifiers 

class UtsNamespace
{
public:

	// Destructor
	//
	~UtsNamespace()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new UtsNamespace instance
	static std::shared_ptr<UtsNamespace> Create(void);
	static std::shared_ptr<UtsNamespace> Create(const std::shared_ptr<UtsNamespace>& utsns);

	//-------------------------------------------------------------------------
	// Properties

	// DomainName
	//
	// Gets/sets the domain name string
	__declspec(property(get=getDomainName, put=putDomainName)) std::string DomainName;
	std::string getDomainName(void);
	void putDomainName(const std::string& value);
	void putDomainName(const char_t* value);

	// HostName
	//
	// Gets/sets the host name string
	__declspec(property(get=getHostName, put=putHostName)) std::string HostName;
	std::string getHostName(void);
	void putHostName(const std::string& value);
	void putHostName(const char_t* value);

private:

	UtsNamespace(const UtsNamespace&)=delete;
	UtsNamespace& operator=(const UtsNamespace&)=delete;

	// Instance Constructors
	//
	UtsNamespace()=default;
	UtsNamespace(const std::string& hostname, const std::string& domainname);
	friend class std::_Ref_count_obj<UtsNamespace>;

	//-------------------------------------------------------------------------
	// Member Variables

	sync::critical_section			m_lock;				// Synchronization object
	std::string						m_hostname;			// Host name string
	std::string						m_domainname;		// Domain name string
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __UTSNAMESPACE_H_
