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

#ifndef __HOST_H_
#define __HOST_H_
#pragma once

#include <memory>
#include "Architecture.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

// Forward Declarations
//
class NativeProcess;

//-----------------------------------------------------------------------------
// Class Host
//
//	todo: words, this sits between Process and the real process

class Host
{
public:

	// Destructor
	//
	~Host();

	//-------------------------------------------------------------------------
	// Member Functions

	// FromNativeProcess (static)
	//
	// Constructs a new Host instance from an existing NativeProcess instance
	static std::unique_ptr<Host> FromNativeProcess(std::unique_ptr<NativeProcess> nativeproc);

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture of the native host process
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	enum class Architecture getArchitecture(void) const;

private:

	Host(const Host&)=delete;
	Host& operator=(const Host&)=delete;

	// nativeproc_t
	//
	// NativeProcess unique pointer
	using nativeproc_t = std::unique_ptr<NativeProcess>;

	// Instance Constructor
	//
	Host(nativeproc_t nativeproc);
	friend std::unique_ptr<Host> std::make_unique<Host, nativeproc_t>(nativeproc_t&&);

	//-------------------------------------------------------------------------
	// Member Variables

	const nativeproc_t			m_nativeproc;		// Native process instance
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOST_H_
