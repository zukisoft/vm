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
using System.CodeDom.Compiler;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using zuki.tools.llvm.clang;
using zuki.tools.llvm.clang.extensions;

using SysFile = System.IO.File;
using ClangFile = zuki.tools.llvm.clang.File;
using ClangType = zuki.tools.llvm.clang.Type;

namespace zuki.vm.tools
{
	static class UapiHeader
	{
		/// <summary>
		/// Converts a TranslationUnit instance into a single header file that contains all
		/// of the declarations and macro definitions.  Macro definitions and enumeration
		/// constants are prefixed with "UAPI_", declarations are prefixed with "uapi_"
		/// </summary>
		/// <param name="transunit">TranslationUnit to be converted</param>
		/// <param name="clangargs">Command line arguments passed into clang</param>
		/// <param name="outheader">Output header file</param>
		public static void Generate(TranslationUnit transunit, IEnumerable<string> clangargs, string outheader)
		{
			if (transunit == null) throw new ArgumentNullException("transunit");
			if (String.IsNullOrEmpty(outheader)) throw new ArgumentNullException("outheader");
			if (!Directory.Exists(Path.GetDirectoryName(outheader))) throw new DirectoryNotFoundException(Path.GetDirectoryName(outheader));
			if (SysFile.Exists(outheader)) SysFile.Delete(outheader);

			// Set up the preamble template
			UapiHeaderPreamble preamble = new UapiHeaderPreamble();
			preamble.ClangArguments = clangargs;
			preamble.HeaderName = Path.GetFileName(outheader);

			// Set up the epilogue template
			UapiHeaderEpilogue epilogue = new UapiHeaderEpilogue();
			epilogue.HeaderName = Path.GetFileName(outheader);

			// Create the dictionary of name mappings; when dealing with macro definitions it's easier
			// to search and replace strings than it is to try and determine if something should be 
			// renamed during output processing
			var namemappings = CreateNameMappings(transunit);

			// Create an IndentedTextWriter instance to control the output
			using (IndentedTextWriter writer = new IndentedTextWriter(SysFile.CreateText(outheader)))
			{
				writer.Write(preamble.TransformText());

				// Process all of the top-level cursors within the translation unit
				transunit.Cursor.EnumerateChildren((cursor, parent) =>
				{
					// Skip over anonymous declarations that are not enumerations
					if (String.IsNullOrEmpty(cursor.DisplayName) && (cursor.Kind != CursorKind.EnumDecl)) return EnumerateChildrenResult.Continue;

					// DECLARATION
					//
					if (cursor.Kind.IsDeclaration)
					{
						// Write the original location of the declaration for reference
						writer.WriteLine("// " + cursor.Location.ToString());

						// Process enumeration, structure, typedef and union declarations
						if (cursor.Kind == CursorKind.EnumDecl) EmitEnum(writer, cursor);
						else if (cursor.Kind == CursorKind.StructDecl) EmitStruct(writer, cursor);
						else if (cursor.Kind == CursorKind.TypedefDecl) EmitTypedef(writer, cursor);
						else if (cursor.Kind == CursorKind.UnionDecl) EmitUnion(writer, cursor);

						// Apply a static_assert in the output to verify that the compiled size of
						// structures, typedefs and unions matches what the clang API calculated
						if ((cursor.Kind != CursorKind.EnumDecl) && (cursor.Type.Size != null))
						{
							writer.WriteLine();
							writer.WriteLine("#if !defined(__midl)");
							writer.WriteLine("static_assert(alignof(uapi_" + cursor.DisplayName + ") == " + cursor.Type.Alignment.ToString() +
								", \"uapi_" + cursor.DisplayName + ": incorrect alignment\");");
							writer.WriteLine("static_assert(sizeof(uapi_" + cursor.DisplayName + ") == " + cursor.Type.Size.ToString() +
								", \"uapi_" + cursor.DisplayName + ": incorrect size\");");
							writer.WriteLine("#endif");
						}

						writer.WriteLine();
					}

					// MACRO DEFINITION
					//
					else if ((cursor.Kind == CursorKind.MacroDefinition) && (!ClangFile.IsNull(cursor.Location.File)))
					{
						// Write the original location of the definition for reference and emit the macro
						writer.WriteLine("// " + cursor.Location.ToString());
						EmitMacroDefinition(writer, cursor, namemappings);
						writer.WriteLine();
					}

					return EnumerateChildrenResult.Continue;
				});

				writer.Write(epilogue.TransformText());
				writer.Flush();
			}
		}

		/// <summary>
		/// Creates a dictionary of current->new declaration names, it was prohibitive to try and do
		/// this inline when processing macros in the translation unit as clang doesn't annotate macro
		/// expansion tokens separately.  A string search/replace is required instead (slow and ugly)
		/// </summary>
		/// <param name="transunit">TranslationUnit instance</param>
		private static Dictionary<string, string> CreateNameMappings(TranslationUnit transunit)
		{
			Dictionary<string, string> dictionary = new Dictionary<string, string>();

			// MACRO DEFINITIONS (UAPI_)
			//
			foreach (var macrodecl in transunit.Cursor.FindChildren((c, p) => (c.Kind == CursorKind.MacroDefinition) && (!ClangFile.IsNull(c.Location.File))).Select(t => t.Item1))
			{
				if(!String.IsNullOrEmpty(macrodecl.DisplayName)) dictionary.Add(macrodecl.DisplayName, "UAPI_" + macrodecl.DisplayName);
			}

			// ENUMERATION CONSTANTS (UAPI_)
			//
			foreach(Cursor enumdecl in transunit.Cursor.FindChildren((c, p) => c.Kind == CursorKind.EnumDecl).Select(t => t.Item1))
			{
				var constants = enumdecl.FindChildren((c, p) => c.Kind == CursorKind.EnumConstantDecl).Select(t => t.Item1);
				foreach(Cursor constant in constants) dictionary.Add(constant.DisplayName, "UAPI_" + constant.DisplayName);
			}

			// DECLARATIONS (uapi_)
			//
			foreach (Cursor decl in transunit.Cursor.FindChildren((c, p) => c.Kind.IsDeclaration).Select(t => t.Item1))
			{
				// Declaration names override macro and enumeration constant names
				if (!String.IsNullOrEmpty(decl.DisplayName)) dictionary[decl.DisplayName] = "uapi_" + decl.DisplayName;
			}

			return dictionary;
		}

		/// <summary>
		/// Emits an enumeration declaration
		/// </summary>
		/// <param name="writer">Text writer instance</param>
		/// <param name="cursor">Enumeration cursor to be emitted</param>
		private static void EmitEnum(IndentedTextWriter writer, Cursor cursor)
		{
			if (writer == null) throw new ArgumentNullException("writer");
			if (cursor == null) throw new ArgumentNullException("cursor");
			if (cursor.Kind != CursorKind.EnumDecl) throw new Exception("EmitEnum: Unexpected cursor kind");

			// Like structs and unions, enums can be anonymous and not have a display name
			writer.Write("enum ");
			if (!String.IsNullOrEmpty(cursor.DisplayName)) writer.Write("uapi_" + cursor.DisplayName + " ");
			writer.WriteLine("{");
			writer.WriteLine();

			// The only valid children of a enumeration declaration are enumeration constant declarations
			writer.Indent++;
			foreach (Cursor child in cursor.FindChildren((c, p) => c.Kind == CursorKind.EnumConstantDecl).Select(t => t.Item1)) EmitEnumConstant(writer, child);
			writer.Indent--;

			writer.WriteLine();
			writer.WriteLine("};");
		}

		/// <summary>
		/// Emits an enumeration constant declaration
		/// </summary>
		/// <param name="writer">Text writer instance</param>
		/// <param name="cursor">Enumeration constant cursor to be emitted</param>
		private static void EmitEnumConstant(IndentedTextWriter writer, Cursor cursor)
		{
			if (writer == null) throw new ArgumentNullException("writer");
			if (cursor == null) throw new ArgumentNullException("cursor");
			if (cursor.Kind != CursorKind.EnumConstantDecl) throw new Exception("EmitEnumConstant: Unexpected cursor kind");

			// Enumeration constants lie in the global namespace, so they need to be prefixed with UAPI_
			writer.WriteLine("UAPI_" + cursor.DisplayName + " = " + cursor.EnumConstant.ToString() + ",");
		}

		/// <summary>
		/// Emits a field declaration
		/// </summary>
		/// <param name="writer">Text writer instance</param>
		/// <param name="cursor">Field declaration cursor to be emitted</param>
		private static void EmitField(IndentedTextWriter writer, Cursor cursor)
		{
			if (writer == null) throw new ArgumentNullException("writer");
			if (cursor == null) throw new ArgumentNullException("cursor");
			if (cursor.Kind != CursorKind.FieldDecl) throw new Exception("EmitField: Unexpected cursor kind");

			// Determine the field type and get the declaration cursor for that type
			ClangType type = (cursor.Type.Kind == TypeKind.ConstantArray) ? cursor.Type.ArrayElementType : cursor.Type;

			// Check for anonymous struct / union being declared through the field
			if (String.IsNullOrEmpty(type.DeclarationCursor.DisplayName) && ((type.DeclarationCursor.Kind == CursorKind.StructDecl) || (type.DeclarationCursor.Kind == CursorKind.UnionDecl)))
			{
				if (type.DeclarationCursor.Kind == CursorKind.StructDecl) EmitStruct(writer, type.DeclarationCursor);
				else if (type.DeclarationCursor.Kind == CursorKind.UnionDecl) EmitUnion(writer, type.DeclarationCursor);
				else throw new Exception("Unexpected anonymous type dectected for field " + cursor.DisplayName);
			}

			// Not an anonymous struct/union, just spit out the data type
			else EmitType(writer, type);
			
			// Field names do not receive a prefix, the display names are preserved
			writer.Write(" " + cursor.DisplayName);

			// ConstantArray fields require the element size suffix, this includes anonymous structs and unions
			if (cursor.Type.Kind == TypeKind.ConstantArray) writer.Write("[" + cursor.Type.ArraySize.ToString() + "]");

			// Fields that have a bit width require that width to be placed after the field name
			if (!(cursor.FieldBitWidth == null)) writer.Write(" : " + cursor.FieldBitWidth.ToString());
			
			writer.WriteLine(";");
		}

		/// <summary>
		/// Emits a macro definition.  The text of the macro uses a search and replace mechanism to deal with 
		/// code elements that have been renamed, it's seemingly not possible to do it any other way as the
		/// clang tokenizer will annotate each token as part of the macro definition
		/// </summary>
		/// <param name="writer">Text writer instance</param>
		/// <param name="cursor">Macro definition cursor to be emitted</param>
		/// <param name="namemappings">Dictionary of current->new code element names</param>
		private static void EmitMacroDefinition(IndentedTextWriter writer, Cursor cursor, Dictionary<string, string> namemappings)
		{
			if (writer == null) throw new ArgumentNullException("writer");
			if (cursor == null) throw new ArgumentNullException("cursor");
			if (namemappings == null) throw new ArgumentNullException("macronames");
			if (cursor.Kind != CursorKind.MacroDefinition) throw new Exception("Unexpected cursor kind");

			// Macro names are prefixed with UAPI_
			writer.Write("#define UAPI_" + cursor.DisplayName);

			// Tokenize the macro so that each token string can be remapped if necessary
			using (var tokens = cursor.Extent.GetTokens())
			{
				List<string> strings = new List<string>();
				foreach (var token in tokens.Skip(1))
				{
					if (namemappings.ContainsKey(token.Spelling)) strings.Add(namemappings[token.Spelling]);
					else strings.Add(token.Spelling);
				}

				// Write the modified collection of token strings into the output file
				string str = String.Join(" ", strings).Trim();
				if (!String.IsNullOrEmpty(str)) writer.Write(" " + str);
			}

			writer.WriteLine();
		}

		/// <summary>
		/// Emits a structure declaration
		/// </summary>
		/// <param name="writer">Text writer instance</param>
		/// <param name="cursor">Structure declaration to be emitted</param>
		private static void EmitStruct(IndentedTextWriter writer, Cursor cursor)
		{
			if (writer == null) throw new ArgumentNullException("writer");
			if (cursor == null) throw new ArgumentNullException("cursor");
			if (cursor.Kind != CursorKind.StructDecl) throw new Exception("EmitStruct: Unexpected cursor kind");

			// Determine if this is an anonymous structure, which implies that it can only appear as
			// part of another struct/union/typedef and won't get a terminating semi-colon
			bool anonymous = String.IsNullOrEmpty(cursor.DisplayName);

			// FORWARD DECLARATION
			//
			if ((!anonymous) && (cursor.Type.Size == null))
			{
				writer.WriteLine("struct " + cursor.DisplayName + ";");
				return;
			}

			// Some unions require a packing override, it appears that clang exposes it properly via alignment
			if (!anonymous) writer.WriteLine("#pragma pack(push, " + cursor.Type.Alignment.ToString() + ")");

			// Named structure declarations receive the lowercase uapi_ prefix
			writer.Write("struct ");
			if (!String.IsNullOrEmpty(cursor.DisplayName)) writer.Write("uapi_" + cursor.DisplayName + " ");
			writer.WriteLine("{");
			writer.WriteLine();

			// The only valid children of a structure declaration are field declarations
			writer.Indent++;
			foreach (Cursor child in cursor.FindChildren((c, p) => c.Kind == CursorKind.FieldDecl).Select(t => t.Item1)) EmitField(writer, child);
			writer.Indent--;

			writer.WriteLine();

			// Anonymous structures are left unterminated, named structures are terminated
			if (anonymous) writer.Write("} ");
			else writer.WriteLine("};");

			// Close out the #pragma pack for non-anonymous structures
			if(!anonymous) writer.WriteLine("#pragma pack(pop)");
		}

		/// <summary>
		/// Emits a type name; if the type is a global declaration the "uapi_" prefix is added to it
		/// </summary>
		/// <param name="writer">Text writer instance</param>
		/// <param name="type">Type name to be emitted</param>
		private static void EmitType(IndentedTextWriter writer, ClangType type)
		{
			if (writer == null) throw new ArgumentNullException("writer");
			if (type == null) throw new ArgumentNullException("type");

			string suffix = String.Empty;

			// POINTER TYPE
			//
			if (type.Kind == TypeKind.Pointer)
			{
				suffix = " *";
				type = type.PointeeType;
			}

			// BUILT-IN TYPE
			//
			if (ClangFile.IsNull(type.DeclarationCursor.Location.File))
			{
				// Built-in types just get emitted as written in the source file
				writer.Write(type.Spelling + suffix);
				return;
			}

			// Prefix struct and union types with the keywords 'struct' and 'union'
			if (type.DeclarationCursor.Kind == CursorKind.StructDecl) writer.Write("struct ");
			else if (type.DeclarationCursor.Kind == CursorKind.UnionDecl) writer.Write("union ");

			// Prefix global declarations with uapi_
			if (type.DeclarationCursor.SemanticParentCursor.Kind == CursorKind.TranslationUnit) writer.Write("uapi_");

			writer.Write(type.DeclarationCursor.Spelling + suffix);
		}

		/// <summary>
		/// Emits a typedef declaration
		/// </summary>
		/// <param name="writer">Text writer instance</param>
		/// <param name="cursor">typedef declaration to be emitted</param>
		private static void EmitTypedef(IndentedTextWriter writer, Cursor cursor)
		{
			if (writer == null) throw new ArgumentNullException("writer");
			if (cursor == null) throw new ArgumentNullException("cursor");
			if (cursor.Kind != CursorKind.TypedefDecl) throw new Exception("EmitTypedef: Unexpected cursor kind");

			ClangType type = cursor.UnderlyingTypedefType;

			writer.Write("typedef ");

			// ANONYMOUS STRUCT / UNION
			//
			if (String.IsNullOrEmpty(type.DeclarationCursor.DisplayName) && ((type.DeclarationCursor.Kind == CursorKind.StructDecl) || (type.DeclarationCursor.Kind == CursorKind.UnionDecl)))
			{
				if (type.DeclarationCursor.Kind == CursorKind.StructDecl) EmitStruct(writer, type.DeclarationCursor);
				else if (type.DeclarationCursor.Kind == CursorKind.UnionDecl) EmitUnion(writer, type.DeclarationCursor);
				else throw new Exception("Unexpected anonymous type dectected for typedef uapi_" + cursor.DisplayName);

				writer.WriteLine("uapi_" + cursor.DisplayName + ";");
			}

			// FUNCTION POINTER
			//
			else if (type.Spelling.Contains("("))
			{
				writer.WriteLine(type.Spelling.Replace("*", "* uapi_" + cursor.DisplayName) + ";");
			}

			// CONSTANT ARRAY
			//
			else if (type.Kind == TypeKind.ConstantArray)
			{
				EmitType(writer, type.ArrayElementType);
				writer.WriteLine(" uapi_" + cursor.DisplayName + "[" + type.ArraySize.ToString() + "];");
			}

			// ANYTHING ELSE
			//
			else
			{
				EmitType(writer, type);
				writer.WriteLine(" uapi_" + cursor.DisplayName + ";");
			}
		}

		/// <summary>
		/// Emits a union declaration
		/// </summary>
		/// <param name="writer">Text writer instance</param>
		/// <param name="cursor">Union declaration to be emitted</param>
		private static void EmitUnion(IndentedTextWriter writer, Cursor cursor)
		{
			if (writer == null) throw new ArgumentNullException("writer");
			if (cursor == null) throw new ArgumentNullException("cursor");
			if (cursor.Kind != CursorKind.UnionDecl) throw new Exception("EmiUnion: Unexpected cursor kind");

			// Determine if this is an anonymous union, which implies that it can only appear as
			// part of another struct/union/typedef and won't get a terminating semi-colon
			bool anonymous = String.IsNullOrEmpty(cursor.DisplayName);

			// FORWARD DECLARATION
			//
			if ((!anonymous) && (cursor.Type.Size == null))
			{
				writer.WriteLine("union " + cursor.DisplayName + ";");
				return;
			}

			// Some unions require a packing override, it appears that clang exposes it properly via alignment
			if (!anonymous) writer.WriteLine("#pragma pack(push, " + cursor.Type.Alignment.ToString() + ")");

			// Named union declarations receive the lowercase uapi_ prefix
			writer.Write("union ");
			if (!String.IsNullOrEmpty(cursor.DisplayName)) writer.Write("uapi_" + cursor.DisplayName + " ");
			writer.WriteLine("{");
			writer.WriteLine();

			// The only valid children of a union declaration are field declarations
			writer.Indent++;
			foreach (Cursor child in cursor.FindChildren((c, p) => c.Kind == CursorKind.FieldDecl).Select(t => t.Item1)) EmitField(writer, child);
			writer.Indent--;

			writer.WriteLine();

			// Anonymous unions are left unterminated, named unions are terminated
			if (anonymous) writer.Write("} ");
			else writer.WriteLine("};");

			// Close out the #pragma pack for non-anonymous structures
			if (!anonymous) writer.WriteLine("#pragma pack(pop)");
		}
	}
}
