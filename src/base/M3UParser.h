/* M3UParser.h

   Copyright (C) 2014 - 2018 Marc Postema (mpostema09 -at- gmail.com)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
   Or, point your browser to http://www.gnu.org/copyleft/gpl.html
*/
#ifndef BASE_M3UPARSER_H_INCLUDE
#define BASE_M3UPARSER_H_INCLUDE BASE_M3UPARSER_H_INCLUDE

#include <Log.h>
#include <base/Tokenizer.h>

#include <fstream>
#include <map>

namespace base {

	/// The class @c M3UParser can be used to parse M3U files with 'satip-freq' extensions.
	class M3UParser {
		public:
			using TransformationMap = std::map<double, std::string>;

			// =======================================================================
			//  -- Constructors and destructor ---------------------------------------
			// =======================================================================
			M3UParser();

			virtual ~M3UParser();

			// =======================================================================
			//  -- Other member functions --------------------------------------------
			// =======================================================================

		public:

			/// Pars the M3U mapping file
			/// @param filePath
			/// @retval true specifies the file is mapped false means something
			/// is wrong with the file
			bool parse(const std::string &filePath);

			/// Get the uri for the requested frequency
			/// @param freq  requested frequency to transform
			/// @param uri   the uri is freq is available
			/// @retval
			bool findURIFor(double freq, std::string &uri) const;

			/// Check if the requested frequency can be transformed
			/// @retval true means the frequency can be used for
			/// transformation
			bool exist(const double freq) const;

			// =======================================================================
			// -- Data members -------------------------------------------------------
			// =======================================================================

		private:

			TransformationMap _transformationMap;
	};

} // namespace base

#endif // BASE_M3UPARSER_H_INCLUDE
