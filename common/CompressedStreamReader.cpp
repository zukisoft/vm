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

#include "stdafx.h"						// Include project pre-compiled headers
#include "CompressedStreamReader.h"		// Include CompressedStreamReader declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

// UINT8_C redefinition
//
#pragma push_macro("UINT8_C")
#undef UINT8_C
#define UINT8_C(x)	static_cast<uint8_t>(x)

//-----------------------------------------------------------------------------

CompressedStreamReader::CompressedStreamReader(const std::unique_ptr<File>& file, size_t offset, size_t length)
{
	if(file == nullptr) throw Exception(E_INVALIDARG);

	// Create a read-only memory mapped view of the provided file object
	m_view = MappedFileView::Create(MappedFile::CreateFromFile(file), FILE_MAP_READ, offset, length);

	// GZIP
	if(CheckMagic(m_view->Pointer, m_view->Length, UINT8_C(0x1F), UINT8_C(0x8B), UINT8_C(0x08), UINT8_C(0x00))) 
		m_stream = std::make_unique<GZipStreamReader>(m_view->Pointer, m_view->Length);

	// XZ
	else if(CheckMagic(m_view->Pointer, m_view->Length, UINT8_C(0xFD), '7', 'z', 'X', 'Z', UINT8_C(0x00))) 
		m_stream = std::make_unique<XzStreamReader>(m_view->Pointer, m_view->Length);

	// BZIP2
	else if(CheckMagic(m_view->Pointer, m_view->Length, 'B', 'Z', 'h')) 
		m_stream = std::make_unique<BZip2StreamReader>(m_view->Pointer, m_view->Length);

	// LZMA
	else if(CheckMagic(m_view->Pointer, m_view->Length, UINT8_C(0x5D), UINT8_C(0x00), UINT8_C(0x00), UINT8_C(0x00))) 
		m_stream = std::make_unique<LzmaStreamReader>(m_view->Pointer, m_view->Length);
	
	// LZOP
	else if(CheckMagic(m_view->Pointer, m_view->Length, UINT8_C(0x89), 'L', 'Z', 'O', UINT8_C(0x00), UINT8_C(0x0D), UINT8_C(0x0A), UINT8_C(0x1A), UINT8_C(0x0A))) 
		m_stream = std::make_unique<LzopStreamReader>(m_view->Pointer, m_view->Length);

	// LZ4
	else if(CheckMagic(m_view->Pointer, m_view->Length, UINT8_C(0x02), UINT8_C(0x21), UINT8_C(0x4C), UINT8_C(0x18))) 
		m_stream = std::make_unique<Lz4StreamReader>(m_view->Pointer, m_view->Length);

	// UNKNOWN OR UNCOMPRESSED
	else m_stream = std::make_unique<BufferStreamReader>(m_view->Pointer, m_view->Length);
}

//-----------------------------------------------------------------------------

#pragma pop_macro("UINT8_C")

#pragma warning(pop)
