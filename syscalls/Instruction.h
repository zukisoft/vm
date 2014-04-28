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

#ifndef __INSTRUCTION_H_
#define __INSTRUCTION_H_
#pragma once

#include "ContextRecord.h"				// Include ContextRecord declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Type Declarations

// instruction_t
//
// Defines the lambda function that executes an Instruction instance
typedef std::function<bool(ContextRecord&)> instruction_t;

//-----------------------------------------------------------------------------
// Instruction
//
// Defines a machine instruction, used to simulate actions that have failed
// and were caught by a vectored exception handler

class Instruction 
{
public:

	// Instance Constructors
	//
	Instruction(uint8_t opcode0, instruction_t executor)
		: m_opcount(1), m_opcode0(opcode0), m_opcode1(0), m_opcode2(0), m_opcode3(0), m_opcode4(0), m_opcode5(0), m_opcode6(0), m_executor(executor) {}
	Instruction(uint8_t opcode0, uint8_t opcode1, instruction_t executor)
		: m_opcount(2), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(0), m_opcode3(0), m_opcode4(0), m_opcode5(0), m_opcode6(0), m_executor(executor) {}
	Instruction(uint8_t opcode0, uint8_t opcode1, uint8_t opcode2, instruction_t executor)
		: m_opcount(3), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(opcode2), m_opcode3(0), m_opcode4(0), m_opcode5(0), m_opcode6(0), m_executor(executor) {}
	Instruction(uint8_t opcode0, uint8_t opcode1, uint8_t opcode2, uint8_t opcode3, instruction_t executor)
		: m_opcount(4), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(opcode2), m_opcode3(opcode3), m_opcode4(0), m_opcode5(0), m_opcode6(0), m_executor(executor) {}
	Instruction(uint8_t opcode0, uint8_t opcode1, uint8_t opcode2, uint8_t opcode3, uint8_t opcode4, instruction_t executor)
		: m_opcount(5), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(opcode2), m_opcode3(opcode3), m_opcode4(opcode4), m_opcode5(0), m_opcode6(0), m_executor(executor) {}
	Instruction(uint8_t opcode0, uint8_t opcode1, uint8_t opcode2, uint8_t opcode3, uint8_t opcode4, uint8_t opcode5, instruction_t executor)
		: m_opcount(6), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(opcode2), m_opcode3(opcode3), m_opcode4(opcode4), m_opcode5(opcode5), m_opcode6(0), m_executor(executor) {}
	Instruction(uint8_t opcode0, uint8_t opcode1, uint8_t opcode2, uint8_t opcode3, uint8_t opcode4, uint8_t opcode5, uint8_t opcode6, instruction_t executor)
		: m_opcount(7), m_opcode0(opcode0), m_opcode1(opcode1), m_opcode2(opcode2), m_opcode3(opcode3), m_opcode4(opcode4), m_opcode5(opcode5), m_opcode6(opcode6), m_executor(executor) {}

	//-------------------------------------------------------------------------
	// Member Functions
	
	// Execute
	//
	// Attempts to execute the defined instruction
	bool Execute(ContextRecord& context);

private:

	Instruction(const Instruction&)=delete;
	Instruction& operator=(const Instruction&)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	uint8_t				m_opcode0;				// Opcode byte 0
	uint8_t				m_opcode1;				// Opcode byte 1
	uint8_t				m_opcode2;				// Opcode byte 2
	uint8_t				m_opcode3;				// Opcode byte 3
	uint8_t				m_opcode4;				// Opcode byte 4
	uint8_t				m_opcode5;				// Opcode byte 5
	uint8_t				m_opcode6;				// Opcode byte 6
	uint8_t				m_opcount;				// Number of opcode bytes
	instruction_t		m_executor;				// Function to execute instruction
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __INSTRUCTION_H_
