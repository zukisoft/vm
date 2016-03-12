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

using System.IO;

namespace zuki.vm.tools
{
	/// <summary>
	/// Specialization for Epilogue runtime text template
	/// </summary>
	public partial class Epilogue
	{
		/// <summary>
		/// Instance constructor
		/// </summary>
		/// <param name="filename">Output file name</param>
		public Epilogue(string filename)
		{
			// Convert the filename into an include guard identifier
			m_includeguard = "__" + Path.GetFileName(filename).Replace('.', '_').ToUpper() + "_";
		}

		/// <summary>
		/// Multiple inclusion guard identifier name
		/// </summary>
		private string m_includeguard;
	}
}
