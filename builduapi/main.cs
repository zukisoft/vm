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
using System.IO;
using zuki.tools.llvm.clang;
using zuki.tools.llvm.clang.extensions;

using ClangFile = zuki.tools.llvm.clang.File;
using SysFile = System.IO.File;

namespace zuki.vm.tools
{
	class main
	{
		static void ShowUsage()
		{
			Console.WriteLine("todo");
		}

		/// <summary>
		/// Main application entry point
		/// </summary>
		/// <param name="args">Command-line arguments</param>
		static void Main(string[] args)
		{
			CommandLine commandline = new CommandLine(args);

			// arg[0] --> input translation unit file
			string infile = @"D:\\uapi.c";
			if (!SysFile.Exists(infile)) { /* todo: throw */ } 

			// arg[1] --> output header file
			string outfile = @"D:\\uapi.h";
			if (!Directory.Exists(Path.GetDirectoryName(outfile))) { /* todo: throw */ }

			// -i:<directory> : original linux header file path (defaults to input file directory)
			string originalheaders = @"D:\kernel-headers\linux-4.3.5";

			// -m:<directory> : modified linux header file path (defaults to none)
			string modifiedheaders = @"";

			// Build the collection of override files for the translation unit
			var virtualfiles = CollectVirtualFiles(modifiedheaders, originalheaders);

			// Build the command line arguments to pass into the clang frontend
			List<string> arguments = new List<string>();
			arguments.Add("-E");
			arguments.Add("-I" + originalheaders);

			// Create a clang translation unit using the provided source file and specifying all
			// modified headers as virtual files to override any matching physical headers
			using (TranslationUnit tu = Clang.CreateTranslationUnit(infile, arguments, virtualfiles, TranslationUnitParseOptions.DetailedPreprocessingRecord))
			{
				bool hasfatal = false;				// Flag indicating a fatal error occurred

				// Dump the translation unit diagnostics to the console before continuing
				foreach (var diagnostic in tu.Diagnostics)
				{
					if (diagnostic.Severity == DiagnosticSeverity.Fatal) hasfatal = true;
					Console.WriteLine(diagnostic.Format(DiagnosticDisplayOptions.DisplaySourceLocation));
					Console.WriteLine();
				}

				// If any of the translation unit diagnostics were a fatal error, stop now
				if (hasfatal) return;

				// Create the output file, overwriting any existing file
				using (StreamWriter writer = SysFile.CreateText(outfile))
				{
					// Generate and emit the preamble
					writer.Write(new Preamble(outfile).TransformText());

					// Enumerate the direct descendants of the translation unit
					tu.Cursor.EnumerateChildren((cursor, parent) =>
					{
						EmitTranslationUnitCursor(writer, cursor, parent);
						return EnumerateChildrenResult.Continue;
					});

					// Generate and emit the epilogue
					writer.Write(new Epilogue(outfile).TransformText());
					writer.Flush();
				}
			}
		}

		/// <summary>
		/// Builds a collection of VirtualFile objects that override the original headers
		/// </summary>
		/// <param name="virtualbase">Virtual/modified header file base path</param>
		/// <param name="physicalbase">Physical/original header file base path</param>
		/// <returns>Collection of VirtualFile objects for the translation unit</returns>
		static List<VirtualFile> CollectVirtualFiles(string virtualbase, string physicalbase)
		{
			List<VirtualFile> virtualfiles = new List<VirtualFile>();

			// If the input directory does not exist, there can be no override files
			if (!Directory.Exists(virtualbase)) return virtualfiles;

			// Normalize the virtual path (within reason)
			virtualbase = new DirectoryInfo(Path.GetFullPath(virtualbase)).FullName;

			// Use a stack so this doesn't need to be done recursively
			Stack<string> directories = new Stack<string>();
			directories.Push(virtualbase);

			while (directories.Count > 0)
			{
				string current = directories.Pop();

				// Add all subdirectories of this directory for subsequent processing 
				foreach (string subdirectory in Directory.GetDirectories(current)) directories.Push(subdirectory);

				// Create VirtualFile entries for each file in this directory, making the names
				// relative to the physical base path rather than the virtual one
				foreach (string file in Directory.GetFiles(current, "*.h"))
				{
					string filename = Path.Combine(physicalbase, file.Substring(virtualbase.Length + 1));
					virtualfiles.Add(new VirtualFile(filename, SysFile.ReadAllText(file)));
				}
			}

			return virtualfiles;
		}

		/// <summary>
		/// Processes and emits a single translation unit cursor
		/// </summary>
		/// <param name="writer">Output file StreamWriter instance</param>
		/// <param name="cursor">Cursor to be processed</param>
		/// <param name="parent">Parent cursor instance (if available)</param>
		static void EmitTranslationUnitCursor(StreamWriter writer, Cursor cursor, Cursor parent)
		{
			if (writer == null) throw new ArgumentNullException("writer");
			if (cursor == null) throw new ArgumentNullException("cursor");
			if (parent == null) throw new ArgumentNullException("parent");

			// Skip cursors that were not generated from an actual header file
			if (ClangFile.IsNull(cursor.Location.File)) return;

			// todo - dummy
			writer.WriteLine(cursor.DisplayName);
		}
	}
}
