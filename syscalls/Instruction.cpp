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

#include "stdafx.h"
#include "Instruction.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// Instruction::Execute
//
// Executes the instruction by invoking the defined lambda function
//
// Arguments:
//
//	context			- Execution context record

bool Instruction::Execute(ContextRecord& context)
{
	uint8_t* eip = reinterpret_cast<uint8_t*>(context.Registers.EIP);

	// This should work better than a loop; each case falls through
	switch(m_opcount) {

		case 7: if(m_opcode6 != eip[6]) return false;	// fall through
		case 6: if(m_opcode5 != eip[5]) return false;	// fall through
		case 5: if(m_opcode4 != eip[4]) return false;	// fall through
		case 4: if(m_opcode3 != eip[3]) return false;	// fall through
		case 3: if(m_opcode2 != eip[2]) return false;	// fall through
		case 2: if(m_opcode1 != eip[1]) return false;	// fall through
		case 1: if(m_opcode0 != eip[0]) return false;	break;
		default: return false;
	}

	// Move the instruction pointer to the first byte after the opcodes
	context.Registers.EIP += m_opcount;
	
	// If execution of the instruction fails, restore the instruction pointer
	if(m_executor(context)) return true;
	else { context.Registers.EIP = reinterpret_cast<uint32_t>(eip); return false; }
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
