/*
 *  This file is part of dsmmorpg.
 *  
 *  dsmmorpg is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  dsmmorpg is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with dsmmorpg.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EMAGICCLAS_HPP_
#define EMAGICCLAS_HPP_

	#include <string>
	using std::string;
	
	enum eMagicClass
	{
		mc_none = 0,
		mc_potion = 1,
		mc_combat_magic = 2,
		mc_nature_magic = 3,
	};

	inline eMagicClass mcStringToNum(const std::string& s)
	{
		if (s == "mc_none") 				return mc_none;
		else if (s == "mc_potion") 			return mc_potion;
		else if (s == "mc_combat_magic") 			return mc_combat_magic;
		else if (s == "mc_nature_magic") 			return mc_nature_magic;

		return mc_none; // unknown string
	}

	inline bool FromString(const string& s, eMagicClass& e)
	{
		if (s == "mc_none") 				e = mc_none;
		else if (s == "mc_potion") 			e = mc_potion;
		else if (s == "mc_combat_magic") 	e = mc_combat_magic;
		else if (s == "mc_nature_magic") 	e = mc_nature_magic;
		else { return false; }

		return true;
	}

#endif /* EMAGICCLAS_HPP_ */
