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

#ifndef __CONTEXTRECORD_H_
#define __CONTEXTRECORD_H_
#pragma once

#pragma warning(push, 4)				// Enable maximum compiler warnings
#pragma warning(disable:4201)			// Nameless struct/union

//-----------------------------------------------------------------------------
// Type Declarations

// opcode_t
//
// Defines an opcode
struct opcode_t {

	uint8_t		b0;						// instruction byte 0
	uint8_t		b1;						// instruction byte 1
	uint8_t		b2;						// instruction byte 2
	uint8_t		b3;						// instruction byte 3
	uint8_t		b4;						// instruction byte 4
	uint8_t		b5;						// instruction byte 5
	uint8_t		b6;						// instruction byte 6
	uint8_t		modrmmask;				// optional mask for modr/m byte (also set modrm to 1)
	uint8_t		numprefixes : 3;		// 0 - 4; number of prefix bytes
	uint8_t		numopcodes: 2;			// 0 - 3; number of opcode bytes
	uint8_t		modrm : 1;				// 0 - 1; flag if modr/m byte required
	uint8_t		displacement: 1;		// 0 - 1; flag if displacement bytes required
	uint8_t		immediate: 1;			// 0 - 1; flag if immediate bytes required
};

// modrm_t
//
// Union that defines the individual values within a ModR/M byte
union modrm_t {
	
	modrm_t(uint8_t val) : value(val) {}

	struct {

		uint8_t		rm:3;
		uint8_t		reg:3;
		uint8_t		mod:2;
	};

	uint8_t			value;
};

// sib_t
//
// Union that defines the individual values within a SIB Byte
union sib_t {
	
	sib_t(uint8_t val) : value(val) {}

	struct {

		uint8_t		base:3;
		uint8_t		index:3;
		uint8_t		scale:2;
	};

	uint8_t			value;
};

//-----------------------------------------------------------------------------
// ContextRecordFlags
//
// Child class of ContextRecord that provides read-only access to the EFLAGS
// register data of the context record

class ContextRecordFlags
{
friend class ContextRecord;
public:

	//-------------------------------------------------------------------------
	// Properties

	// CF (Carry Flag - bit 0)
	__declspec(property(get=getCF)) bool CF;
	bool getCF(void) const { return (m_eflags & 0x00000001) != 0; }

	// PF (Parity Flag - bit 2)
	__declspec(property(get=getPF)) bool PF;
	bool getPF(void) const { return (m_eflags & 0x00000004) != 0; }

	// AF (Auxiliary Carry Flag - bit 4)
	__declspec(property(get=getAF)) bool AF;
	bool getAF(void) const { return (m_eflags & 0x00000010) != 0; }

	// ZF (Zero Flag - bit 6)
	__declspec(property(get=getZF)) bool ZF;
	bool getZF(void) const { return (m_eflags & 0x00000040) != 0; }

	// SF (Sign Flag - bit 7)
	__declspec(property(get=getSF)) bool SF;
	bool getSF(void) const { return (m_eflags & 0x00000080) != 0; }

	// TF (Trap Flag - bit 8)
	__declspec(property(get=getTF)) bool TF;
	bool getTF(void) const { return (m_eflags & 0x00000100) != 0; }

	// IF (Interrupt Enable Flag - bit 9)
	__declspec(property(get=getIF)) bool IF;
	bool getIF(void) const { return (m_eflags & 0x00000200) != 0; }

	// DF (Direction Flag - bit 10)
	__declspec(property(get=getDF)) bool DF;
	bool getDF(void) const { return (m_eflags & 0x00000400) != 0; }

	// OF (Overflow Flag - bit 11)
	__declspec(property(get=getOF)) bool OF;
	bool getOF(void) const { return (m_eflags & 0x00000800) != 0; }

	// IOPL (I/O Privilege Level - bits 12 and 13)
	__declspec(property(get=getIOPL)) uint8_t IOPL;
	uint8_t getIOPL(void) const { return (m_eflags & 0x00003000) >> 12; }

	// NT (Nested Task Flag - bit 14)
	__declspec(property(get=getNT)) bool NT;
	bool getNT(void) const { return (m_eflags & 0x00004000) != 0; }

	// RF (Resume Flag - bit 16)
	__declspec(property(get=getRF)) bool RF;
	bool getRF(void) const { return (m_eflags & 0x00010000) != 0; }

	// VM (Virtual 8086 Mode - bit 17)
	__declspec(property(get=getVM)) bool VM;
	bool getVM(void) const { return (m_eflags & 0x00020000) != 0; }

	// AC (Alignment Check Flag - bit 18)
	__declspec(property(get=getAC)) bool AC;
	bool getAC(void) const { return (m_eflags & 0x00040000) != 0; }

	// VIF (Virtual Interrupt Flag - bit 19)
	__declspec(property(get=getVIF)) bool VIF;
	bool getVIF(void) const { return (m_eflags & 0x00080000) != 0; }

	// VIP (Virtual Interrupt Pending Flag - bit 20)
	__declspec(property(get=getVIP)) bool VIP;
	bool getVIP(void) const { return (m_eflags & 0x00100000) != 0; }

	// ID (Identification Flag - bit 21)
	__declspec(property(get=getID)) bool ID;
	bool getID(void) const { return (m_eflags & 0x00200000) != 0; }

private:

	// Instance Constructor
	ContextRecordFlags(uint32_t eflags) : m_eflags(eflags) {}

	ContextRecordFlags(const ContextRecordFlags&);
	ContextRecordFlags& operator=(const ContextRecordFlags&);

	//-------------------------------------------------------------------------
	// Member Variables

	uint32_t				m_eflags;			// Value of the EFLAGS register
};

//-----------------------------------------------------------------------------
// ContextRecordRegisters
//
// Child class of ContextRecord that provides access to the context registers

class ContextRecordRegisters
{
friend class ContextRecord;
public:

	//-------------------------------------------------------------------------
	// Properties

	// AH Register
	__declspec(property(get=getAH, put=putAH)) uint8_t AH;
	uint8_t getAH(void) const { return (m_context->Eax & 0x0000FF00) >> 8; }
	void putAH(uint8_t value) const { m_context->Eax &= 0xFFFF00FF; m_context->Eax |= (static_cast<uint16_t>(value) << 8); }

	// AL Register
	__declspec(property(get=getAL, put=putAL)) uint8_t AL;
	uint8_t getAL(void) const { return m_context->Eax & 0x000000FF; }
	void putAL(uint8_t value) const { m_context->Eax &= 0xFFFFFF00; m_context->Eax |= value; }

	// AX Register
	__declspec(property(get=getAX, put=putAX)) uint16_t AX;
	uint16_t getAX(void) const { return m_context->Eax & 0x0000FFFF; }
	void putAX(uint16_t value) const { m_context->Eax &= 0xFFFF0000; m_context->Eax |= value; }

	// BH Register
	__declspec(property(get=getBH, put=putBH)) uint8_t BH;
	uint8_t getBH(void) const { return (m_context->Ebx & 0x0000FF00) >> 8; }
	void putBH(uint8_t value) const { m_context->Ebx &= 0xFFFF00FF; m_context->Ebx |= (static_cast<uint16_t>(value) << 8); }

	// BL Register
	__declspec(property(get=getBL, put=putBL)) uint8_t BL;
	uint8_t getBL(void) const { return m_context->Ebx & 0x000000FF; }
	void putBL(uint8_t value) const { m_context->Ebx &= 0xFFFFFF00; m_context->Ebx |= value; }

	// BP Register
	__declspec(property(get=getBP, put=putBP)) uint16_t BP;
	uint16_t getBP(void) const { return m_context->Ebp & 0x0000FFFF; }
	void putBP(uint16_t value) const { m_context->Ebp &= 0xFFFF0000; m_context->Ebp |= value; }

	// BX Register
	__declspec(property(get=getBX, put=putBX)) uint16_t BX;
	uint16_t getBX(void) const { return m_context->Ebx & 0x0000FFFF; }
	void putBX(uint16_t value) const { m_context->Ebx &= 0xFFFF0000; m_context->Ebx |= value; }

	// CH Register
	__declspec(property(get=getCH, put=putCH)) uint8_t CH;
	uint8_t getCH(void) const { return (m_context->Ecx & 0x0000FF00) >> 8; }
	void putCH(uint8_t value) const { m_context->Ecx &= 0xFFFF00FF; m_context->Ecx |= (static_cast<uint16_t>(value) << 8); }

	// CL Register
	__declspec(property(get=getCL, put=putCL)) uint8_t CL;
	uint8_t getCL(void) const { return m_context->Ecx & 0x000000FF; }
	void putCL(uint8_t value) const { m_context->Ecx &= 0xFFFFFF00; m_context->Ecx |= value; }

	// CS Register
	__declspec(property(get=getCS)) uint16_t CS;
	uint16_t getCS(void) const { return static_cast<uint16_t>(m_context->SegCs); }

	// CX Register
	__declspec(property(get=getCX, put=putCX)) uint16_t CX;
	uint16_t getCX(void) const { return m_context->Ecx & 0x0000FFFF; }
	void putCX(uint16_t value) const { m_context->Ecx &= 0xFFFF0000; m_context->Ecx |= value; }

	// DH Register
	__declspec(property(get=getDH, put=putDH)) uint8_t DH;
	uint8_t getDH(void) const { return (m_context->Edx & 0x0000FF00) >> 8; }
	void putDH(uint8_t value) const { m_context->Edx &= 0xFFFF00FF; m_context->Edx |= (static_cast<uint16_t>(value) << 8); }

	// DI Register
	__declspec(property(get=getDI, put=putDI)) uint16_t DI;
	uint16_t getDI(void) const { return m_context->Edi & 0x0000FFFF; }
	void putDI(uint16_t value) const { m_context->Edi &= 0xFFFF0000; m_context->Edi |= value; }

	// DL Register
	__declspec(property(get=getDL, put=putDL)) uint8_t DL;
	uint8_t getDL(void) const { return m_context->Edx & 0x000000FF; }
	void putDL(uint8_t value) const { m_context->Edx &= 0xFFFFFF00; m_context->Edx |= value; }

	// DS Register
	__declspec(property(get=getDS)) uint16_t CS;
	uint16_t getDS(void) const { return static_cast<uint16_t>(m_context->SegDs); }

	// DX Register
	__declspec(property(get=getDX, put=putDX)) uint16_t DX;
	uint16_t getDX(void) const { return m_context->Edx & 0x0000FFFF; }
	void putDX(uint16_t value) const { m_context->Edx &= 0xFFFF0000; m_context->Edx |= value; }

	// EAX Register
	__declspec(property(get=getEAX, put=putEAX)) uint32_t EAX;
	uint32_t getEAX(void) const { return m_context->Eax; }
	void putEAX(uint32_t value) const { m_context->Eax = value; }
	
	// EBP Register
	__declspec(property(get=getEBP, put=putEBP)) uint32_t EBP;
	uint32_t getEBP(void) const { return m_context->Ebp; }
	void putEBP(uint32_t value) const { m_context->Ebp = value; }

	// EBX Register
	__declspec(property(get=getEBX, put=putEBX)) uint32_t EBX;
	uint32_t getEBX(void) const { return m_context->Ebx; }
	void putEBX(uint32_t value) const { m_context->Ebx = value; }
	
	// ECX Register
	__declspec(property(get=getECX, put=putECX)) uint32_t ECX;
	uint32_t getECX(void) const { return m_context->Ecx; }
	void putECX(uint32_t value) const { m_context->Ecx = value; }
	
	// EDI Register
	__declspec(property(get=getEDI, put=putEDI)) uint32_t EDI;
	uint32_t getEDI(void) const { return m_context->Edi; }
	void putEDI(uint32_t value) const { m_context->Edi = value; }

	// EDX Register
	__declspec(property(get=getEDX, put=putEDX)) uint32_t EDX;
	uint32_t getEDX(void) const { return m_context->Edx; }
	void putEDX(uint32_t value) const { m_context->Edx = value; }

	// EIP Register
	__declspec(property(get=getEIP)) uint32_t EIP;
	uint32_t getEIP(void) const { return m_context->Eip; }

	// ES Register
	__declspec(property(get=getES)) uint16_t CS;
	uint16_t getES(void) const { return static_cast<uint16_t>(m_context->SegEs); }

	// ESI Register
	__declspec(property(get=getESI, put=putESI)) uint32_t ESI;
	uint32_t getESI(void) const { return m_context->Esi; }
	void putESI(uint32_t value) const { m_context->Esi = value; }

	// ESP Register
	__declspec(property(get=getESP, put=putESP)) uint32_t ESP;
	uint32_t getESP(void) const { return m_context->Esp; }
	void putESP(uint32_t value) const { m_context->Esp = value; }

	// FS Register
	__declspec(property(get=getFS)) uint16_t CS;
	uint16_t getFS(void) const { return static_cast<uint16_t>(m_context->SegFs); }

	// GS Register
	__declspec(property(get=getGS)) uint16_t CS;
	uint16_t getGS(void) const { return static_cast<uint16_t>(m_context->SegGs); }

	// SI Register
	__declspec(property(get=getSI, put=putSI)) uint16_t SI;
	uint16_t getSI(void) const { return m_context->Esi & 0x0000FFFF; }
	void putSI(uint16_t value) const { m_context->Esi &= 0xFFFF0000; m_context->Esi |= value; }

	// SP Register
	__declspec(property(get=getSP, put=putSP)) uint16_t SP;
	uint16_t getSP(void) const { return m_context->Esp & 0x0000FFFF; }
	void putSP(uint16_t value) const { m_context->Esp &= 0xFFFF0000; m_context->Esp |= value; }

	// SS Register
	__declspec(property(get=getSS)) uint16_t SS;
	uint16_t getSS(void) const { return static_cast<uint16_t>(m_context->SegSs); }
	
private:

	// Instance Constructor
	ContextRecordRegisters(PCONTEXT context) : m_context(context) {}

	ContextRecordRegisters(const ContextRecordRegisters&);
	ContextRecordRegisters& operator=(const ContextRecordRegisters&);

	//-------------------------------------------------------------------------
	// Member Variables

	PCONTEXT				m_context;			// Pointer to the actual context
};

//-----------------------------------------------------------------------------
// ContextRecord
//
// Wrapper class for the Win32 CONTEXT structure that is used to access and
// manipulate the system state during processing of a vectored exception hander.

class ContextRecord
{
public:

	// Instance Constructor
	ContextRecord(PCONTEXT context) : m_context(context), m_flags(context->EFlags),
		m_registers(context) {}

	//-------------------------------------------------------------------------
	// Member Functions

	// AtInstruction
	//
	// Determines if the context instruction pointer indicates the instruction
	bool AtInstruction(const opcode_t& opcode) const;

	// EatInstruction
	//
	// "Eats" the instruction by advancing the EIP register
	void EatInstruction(const opcode_t& opcode) { EatInstruction(opcode, 0); }
	void EatInstruction(const opcode_t& opcode, size_t extra);

	uint8_t GetOperand8(const opcode_t& opcode);
	uint16_t GetOperand16(const opcode_t& opcode);
	uint32_t GetOperand32(const opcode_t& opcode);

	//-------------------------------------------------------------------------
	// Properties

	// Flags
	//
	// Accesses the contained ContextRecordFlags instance
	__declspec(property(get=getFlags)) const ContextRecordFlags& Flags;
	const ContextRecordFlags& getFlags(void) const { return m_flags; }

	// Registers
	//
	// Accesses the contained ContextRecordRegisters instance
	__declspec(property(get=getRegisters)) const ContextRecordRegisters& Registers;
	const ContextRecordRegisters& getRegisters(void) const { return m_registers; }

private:

	ContextRecord(const ContextRecord&);
	ContextRecord& operator=(const ContextRecord&);

	//-------------------------------------------------------------------------
	// Member Variables

	PCONTEXT				m_context;			// Pointer to the actual context
	ContextRecordFlags		m_flags;			// Contained EFLAGS helper
	ContextRecordRegisters	m_registers;		// Contained registers helper
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __CONTEXTRECORD_H_
