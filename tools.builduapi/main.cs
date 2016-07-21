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

using SysFile = System.IO.File;

namespace zuki.vm.tools
{
	class main
	{
		/// <summary>
		/// Main application entry point
		/// </summary>
		/// <param name="args">Array of command line arguments</param>
		static void Main(string[] args)
		{
			CommandLine commandline = new CommandLine(args);		// Command line parser
			List<string> clangargs = new List<string>();            // Arguments for libclang

			// Simple banner for the output panel
			Console.WriteLine();
			Console.WriteLine("builduapi " + String.Join(" ", args));
			Console.WriteLine();

			try
			{
				// There needs to be at least 2 command line arguments present
				if (commandline.Arguments.Count < 2) throw new ArgumentException("Invalid command line arguments");

				// arg[0] --> input translation unit file
				string infile = commandline.Arguments[0];

				// arg[1] --> output header file
				string outfile = commandline.Arguments[1];

				// -i:<directory> --> include file path
				string includepath = Environment.CurrentDirectory;
				if (commandline.Switches.ContainsKey("i")) includepath = commandline.Switches["i"];
				clangargs.Add("-I" + includepath);

				// -m64 / -x64 --> x64 build target
				if (commandline.Switches.ContainsKey("m64") || commandline.Switches.ContainsKey("x64")) clangargs.Add("-m64");

				// -mx32 / -x32 --> x32 build target
				else if (commandline.Switches.ContainsKey("mx32") || commandline.Switches.ContainsKey("x32")) clangargs.Add("-mx32");

				// -m32 / -x86 --> x86 build target (default)
				else clangargs.Add("-m32");

				// Verify that the input file and include directory exist
				if (!SysFile.Exists(infile)) throw new FileNotFoundException("Translation unit input file not found", infile);
				if (!Directory.Exists(includepath)) throw new DirectoryNotFoundException("Include path " + includepath + " not found");

				// Attempt to create the output directory
				try { Directory.CreateDirectory(Path.GetDirectoryName(outfile)); }
				catch { /* DO NOTHING */ }

				// Ensure that the output directory exists or was successfully created above
				if (!Directory.Exists(Path.GetDirectoryName(outfile)))
					throw new DirectoryNotFoundException("Output file directory " + Path.GetDirectoryName(outfile) + " not found");

				// Create a clang translation unit using the provided information
				using (TranslationUnit transunit = Clang.CreateTranslationUnit(infile, clangargs, TranslationUnitParseOptions.DetailedPreprocessingRecord))
				{
					bool hasfatal = false;              // Flag indicating a fatal error occurred

					// Dump the translation unit diagnostics to the console before continuing
					foreach (var diagnostic in transunit.Diagnostics)
					{
						if (diagnostic.Severity == DiagnosticSeverity.Fatal) hasfatal = true;
						Console.WriteLine(diagnostic.Format(DiagnosticDisplayOptions.DisplaySourceLocation));
						Console.WriteLine();
					}

					// If any of the translation unit diagnostics were a fatal error, stop now
					if (hasfatal) throw new Exception("Fatal error occurred processing input translation unit " + infile);

					// Generate the UAPI header
					UapiHeader.Generate(transunit, clangargs, outfile);
				}
			}

			catch (Exception ex)
			{
				Console.WriteLine();
				Console.WriteLine(">> ERROR: " + ex.Message);
				Console.WriteLine();
			}
		}
	}
}
