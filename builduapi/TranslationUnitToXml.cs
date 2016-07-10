//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using zuki.tools.llvm.clang;
using zuki.tools.llvm.clang.extensions;

using SysFile = System.IO.File;
using ClangFile = zuki.tools.llvm.clang.File;

namespace zuki.vm.tools
{
	/// <summary>
	/// Converts a translation unit into XML
	/// </summary>
	static class TranslationUnitToXml
	{
		/// <summary>
		/// Converts the input translation unit into XML
		/// </summary>
		/// <param name="transunit">Input translation unit</param>
		/// <param name="outxml">Output XML file name</param>
		public static void Transform(TranslationUnit transunit, string outxml)
		{
			if (transunit == null) throw new ArgumentNullException("transunit");
			if (String.IsNullOrEmpty(outxml)) throw new ArgumentNullException("outxml");
			if (!Directory.Exists(Path.GetDirectoryName(outxml))) throw new DirectoryNotFoundException(Path.GetDirectoryName(outxml));
			if (SysFile.Exists(outxml)) SysFile.Delete(outxml);

			XmlWriterSettings xmlsettings = new XmlWriterSettings();
			xmlsettings.Encoding = Encoding.UTF8;
			xmlsettings.Indent = true;
			xmlsettings.IndentChars = "\t";
			xmlsettings.NewLineChars = "\r\n";
			xmlsettings.NewLineHandling = NewLineHandling.Entitize;
			xmlsettings.NewLineOnAttributes = false;

			using (XmlWriter writer = XmlWriter.Create(outxml, xmlsettings))
			{
				writer.WriteStartDocument();
				EmitCursor(writer, transunit.Cursor, Cursor.Null);
				//writer.WriteStartElement("TranslationUnit");

				//// Enumerate the direct descendants of the translation unit
				//transunit.Cursor.EnumerateChildren((cursor, parent) =>
				//{
				//	EmitCursor(writer, cursor, parent);
				//	return EnumerateChildrenResult.Continue;
				//});

				//writer.WriteEndElement();
				writer.Flush();
			}
		}

		public static string GetCursorIdentifier(Cursor cursor)
		{
			return cursor.Location.File.UniqueIdentifier.ToString() + ":" + cursor.Location.Offset.ToString();
		}

		public static void EmitCursor(XmlWriter writer, Cursor cursor, Cursor parent)
		{
			// Non-TranslationUnit cursors that aren't defined by an actual source code file are ignored
			if (cursor.Kind != CursorKind.TranslationUnit && ClangFile.IsNull(cursor.Location.File)) return;

			// EXPRESSION
			//
			// Tokenize and write the entire expression
			if (cursor.Kind.IsExpression)
			{
				writer.WriteStartElement("Expression");

				using (var tokens = cursor.Extent.GetTokens())
				{
					string str = String.Join(" ", tokens).Trim();
					if (!String.IsNullOrEmpty(str)) writer.WriteString(str);
				}

				writer.WriteEndElement();
				return;
			}

			// MACRO DEFINITION
			//
			// Tokenize and write the entire macro definition
			if (cursor.Kind == CursorKind.MacroDefinition)
			{
				writer.WriteStartElement("Macro");
				writer.WriteAttributeString("Name", cursor.DisplayName);

				using (var tokens = cursor.Extent.GetTokens())
				{
					string str = String.Join(" ", tokens.Skip(1)).Trim();
					if (!String.IsNullOrEmpty(str)) //writer.WriteString(str);
						writer.WriteCData(str);
				}

				writer.WriteEndElement();
				return;
			}

			// MACRO EXPANSION
			//
			// Ignored
			if (cursor.Kind == CursorKind.MacroExpansion) return;

			// INCLUSION DIRECTIVE
			//
			// Ignored
			if (cursor.Kind == CursorKind.InclusionDirective) return;

			// FIELD DECLARATION
			//
			//
			if (cursor.Kind == CursorKind.FieldDecl)
			{
				writer.WriteStartElement("Field");
				writer.WriteAttributeString("Name", cursor.DisplayName);
				//writer.WriteAttributeString("Type", cursor.Type.ToString());
				writer.WriteAttributeString("Size", cursor.Type.Size.ToString());

				cursor.EnumerateChildren((c, p) =>
				{
					EmitCursor(writer, c, p);
					return EnumerateChildrenResult.Continue;
				});

				writer.WriteEndElement();
				return;
			}

			if (cursor.Kind == CursorKind.TypedefDecl)
			{
				writer.WriteStartElement("TypedefDecl");
				writer.WriteAttributeString("Name", cursor.DisplayName);
				writer.WriteAttributeString("UnderlyingType", cursor.UnderlyingTypedefType.ToString());

				//cursor.EnumerateChildren((c, p) =>
				//{
				//	EmitCursor(writer, c, p);
				//	return EnumerateChildrenResult.Continue;
				//});

				writer.WriteEndElement();
				return;
			}

			if (cursor.Kind == CursorKind.TypeRef)
			{
				writer.WriteStartElement("TypeRef");

				if (ClangFile.IsNull(cursor.Type.DeclarationCursor.Location.File))
					writer.WriteAttributeString("GLOBAL", "true");

				writer.WriteAttributeString("Name", cursor.DisplayName);
				//writer.WriteAttributeString("UnderlyingType", cursor.ReferencedCursor.Type.ToString());

				writer.WriteEndElement();
				return;
			}


			writer.WriteStartElement(cursor.Kind.ToString("S"));
			writer.WriteAttributeString("Name", cursor.DisplayName);

			// UniqueIdentifier
			//
			//writer.WriteAttributeString("UniqueIdentifier", GetCursorIdentifier(cursor));

			// DisplayName
			//
			//writer.WriteAttributeString("DisplayName", cursor.DisplayName);

			//// Spelling (optional)
			////
			//if(cursor.DisplayName != cursor.Spelling)
			//	writer.WriteAttributeString("Spelling", cursor.Spelling);

			//// Size (optional)
			////
			//long? size = cursor.Type.Size;
			//if (size != null) writer.WriteAttributeString("Size", size.ToString());

			//// ReferenceIdentifier (optional)
			////
			//if (!Cursor.IsNull(cursor.ReferencedCursor))
			//	writer.WriteAttributeString("ReferenceIdentifier", GetCursorIdentifier(cursor.ReferencedCursor));

			////// SPECIAL CASE: MACRO DEFINITION
			//////
			////if (cursor.Kind == CursorKind.MacroDefinition || cursor.Kind == CursorKind.UnexposedExpr)
			////{
			////	using (var tokens = cursor.Extent.GetTokens())
			////	{
			////		string str = String.Join(" ", tokens.Skip(1)).Trim();
			////		if (!String.IsNullOrEmpty(str)) writer.WriteAttributeString("Definition", str);
			////	}
			////}

			//if (cursor.Kind == CursorKind.BinaryOperator)
			//{
			//	using (var tokens = cursor.Extent.GetTokens())
			//	{
			//		// Definition
			//		//
			//		string str = String.Join(" ", tokens.Skip(1).First()).Trim();
			//		if (!String.IsNullOrEmpty(str)) writer.WriteAttributeString("Operator", str);
			//	}
			//}

			//// SPECIAL CASE: TYPEDEF
			////
			//if (cursor.Kind == CursorKind.TypedefDecl)
			//{
			//	writer.WriteAttributeString("UnderlyingTypedefType", cursor.UnderlyingTypedefType.ToString());
			//}

			//if (cursor.Kind == CursorKind.IntegerLiteral)
			//{
			//	///Extent extent = cursor.Location.File.GetExtent(cursor.Extent.Start.Offset, cursor.Extent.End.Offset);

			//	using (var tokens = cursor.Extent.GetTokens())
			//	{
			//		string str = String.Join(" ", tokens).Trim();
			//		if (!String.IsNullOrEmpty(str)) writer.WriteString(str);
			//	}
			//}

			// Enumerate all of the immediate children of this cursor and emit them
			cursor.EnumerateChildren((c, p) =>
			{
				EmitCursor(writer, c, p);
				return EnumerateChildrenResult.Continue;
			});

			writer.WriteEndElement();
		}
	}
}
