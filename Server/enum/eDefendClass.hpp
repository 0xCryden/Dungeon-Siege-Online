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

#ifndef EDEFENDCLASS_HPP_
#define EDEFENDCLASS_HPP_

	#include <string>
	using std::string;
	
	enum eDefendClass
	{
		dc_skin = 0,
		dc_shield = 1,
	};
	
	/*inline string ToString (eAttackClass e)
	{
		switch (e)
		{
			case ac_beastfu:			return "ac_beastfu";
			case ac_axe: 			return "ac_axe";
			case es_feet: 					return "es_feet";
			case es_chest: 					return "es_chest";
			case es_head: 					return "es_head";
			case es_forearms: 				return "es_forearms";
			case es_amulet: 				return "es_amulet";
			case es_spellbook: 				return "es_spellbook";
			case es_ring_0: 				return "es_ring_0";
			case es_ring_1: 				return "es_ring_1";
			case es_ring_2: 				return "es_ring_2";
			case es_ring_3:					return "es_ring_3";
			case es_ring: 					return "es_ring";
			case es_none: 					return "es_none";
			case es_any:					return "es_any";
			
			default: break;
		}
		
		return "es_none";
	}*/

	inline string ToString(eDefendClass dc)
	{
		if (dc == dc_skin)			return "dc_skin";
		else if (dc == dc_shield)	return "dc_shield";

		return "dc_skin";
	}

	inline eDefendClass StringToDc(const std::string& s)
	{
	    if (s == "dc_skin") 			return dc_skin;
	    else if (s == "dc_shield") 		return dc_shield;

	    return dc_skin; // unknown string
	}

	inline bool FromString(const string& s, eDefendClass& e)
	{
		if (s == "dc_skin") 			e = dc_skin;
		else if (s == "dc_shield") 		e = dc_shield;
		else { return false; }

		return true;
	}


#endif /* EDEFENDCLASS_HPP_ */
