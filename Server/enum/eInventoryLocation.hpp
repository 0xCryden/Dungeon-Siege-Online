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

#ifndef EINVENTORYLOCATION_HPP_
#define EINVENTORYLOCATION_HPP_

	#include <string>
	using std::string;

	enum eInventoryLocation
	{
		il_active_melee_weapon 			= 0,
		il_active_ranged_weapon  		= 1,
		il_active_primary_spell  		= 2,
		il_active_secondary_spell  		= 3,
		il_spell_1  					= 4,
		il_spell_2  					= 5,
		il_spell_3 						= 6,
		il_spell_4  					= 7,
		il_spell_5  					= 8,
		il_spell_6  					= 9,
		il_spell_7  					= 10,
		il_spell_8  					= 11,
		il_spell_9  					= 12,
		il_spell_10  					= 13,
		il_spell_11  					= 14,
		il_spell_12  					= 15,
		il_shield   					= 16,
		il_invalid   					= 17,
		il_all    						= 18,
		il_all_active    				= 19,
		il_all_spells    				= 20,
		il_main     					= 21,
	};

	inline string ToString(eInventoryLocation e)
	{
		switch (e)
		{
			case il_active_melee_weapon: 		return "il_active_melee_weapon";
			case il_active_ranged_weapon: 		return "il_active_ranged_weapon";
			case il_active_primary_spell: 		return "il_active_primary_spell";
			case il_active_secondary_spell: 	return "il_active_secondary_spell";
			case il_spell_1: 					return "il_spell_1";
			case il_spell_2: 					return "il_spell_2";
			case il_spell_3: 					return "il_spell_3";
			case il_spell_4: 					return "il_spell_4";
			case il_spell_5: 					return "il_spell_5";
			case il_spell_6: 					return "il_spell_6";
			case il_spell_7: 					return "il_spell_7";
			case il_spell_8: 					return "il_spell_8";
			case il_spell_9: 					return "il_spell_9";
			case il_spell_10: 					return "il_spell_10";
			case il_spell_11: 					return "il_spell_11";
			case il_spell_12: 					return "il_spell_12";
			case il_shield: 					return "il_shield";
			case il_invalid: 					return "il_invalid";
			case il_all: 						return "il_all";
			case il_all_active: 				return "il_all_active";
			case il_all_spells: 				return "il_all_spells";
			case il_main: 						return "il_main";

			default: break;
		}
		return "";
	}

	inline eInventoryLocation StringToNumLoc(const std::string& s)
	{
		if (s == "il_active_melee_weapon") 		return il_active_melee_weapon;
		else if (s == "il_active_ranged_weapon") return il_active_ranged_weapon;
		else if (s == "il_active_primary_spell") return il_active_primary_spell;
		else if (s == "il_active_secondary_spell") return il_active_secondary_spell;
		else if (s == "il_spell_1") 			 return il_spell_1;
		else if (s == "il_spell_2") 			 return il_spell_2;
		else if (s == "il_spell_3") 			 return il_spell_3;
		else if (s == "il_spell_4") 			 return il_spell_4;
		else if (s == "il_spell_5") 			 return il_spell_5;
		else if (s == "il_spell_6") 			 return il_spell_6;
		else if (s == "il_spell_7") 			 return il_spell_7;
		else if (s == "il_spell_8") 			 return il_spell_8;
		else if (s == "il_spell_9") 			 return il_spell_9;
		else if (s == "il_spell_10") 			 return il_spell_10;
		else if (s == "il_spell_11") 			 return il_spell_11;
		else if (s == "il_spell_12") 			 return il_spell_12;
		else if (s == "il_shield") 			 return il_shield;
		else if (s == "il_invalid") 			 return il_invalid;
		else if (s == "il_all") 				 return il_all;
		else if (s == "il_all_active") 		 return il_all_active;
		else if (s == "il_all_spells") 		 return il_all_spells;
		else if (s == "il_main") 				 return il_main;

		return il_invalid; // unknown string
	}

	inline bool FromString(const string& s, eInventoryLocation& e)
	{
		if (s == "il_active_melee_weapon") 		e = il_active_melee_weapon;
		else if (s == "il_active_ranged_weapon") e = il_active_ranged_weapon;
		else if (s == "il_active_primary_spell") e = il_active_primary_spell;
		else if (s == "il_active_secondary_spell") e = il_active_secondary_spell;
		else if (s == "il_spell_1") 			 e = il_spell_1;
		else if (s == "il_spell_2") 			 e = il_spell_2;
		else if (s == "il_spell_3") 			 e = il_spell_3;
		else if (s == "il_spell_4") 			 e = il_spell_4;
		else if (s == "il_spell_5") 			 e = il_spell_5;
		else if (s == "il_spell_6") 			 e = il_spell_6;
		else if (s == "il_spell_7") 			 e = il_spell_7;
		else if (s == "il_spell_8") 			 e = il_spell_8;
		else if (s == "il_spell_9") 			 e = il_spell_9;
		else if (s == "il_spell_10") 			 e = il_spell_10;
		else if (s == "il_spell_11") 			 e = il_spell_11;
		else if (s == "il_spell_12") 			 e = il_spell_12;
		else if (s == "il_shield") 				 e = il_shield;
		else if (s == "il_invalid") 			 e = il_invalid;
		else if (s == "il_all") 				 e = il_all;
		else if (s == "il_all_active") 			 e = il_all_active;
		else if (s == "il_all_spells") 			 e = il_all_spells;
		else if (s == "il_main") 				 e = il_main;
		else { return false; }

		return true;
	}

#endif /* EINVENTORYLOCATION_HPP_ */
