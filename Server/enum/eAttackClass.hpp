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

#ifndef EATTACKCLASS_HPP_
#define EATTACKCLASS_HPP_

	#include <string>
	using std::string;
	
	enum eAttackClass
	{
		ac_invalid = -1,
		ac_beastfu = 0,
		ac_axe = 1,
		ac_club = 2,
	    ac_dagger = 3,
		ac_hammer = 4,
		ac_mace = 5,
		ac_staff = 6,
		ac_sword = 7,
		ac_bow = 8,
		ac_minigun = 9,
		ac_arrow = 10,
		ac_bolt = 11,
		ac_combat_magic = 12,
		ac_nature_magic = 13,
	};
	
	inline string ToString (eAttackClass e)
	{
		switch (e)
		{
			case ac_beastfu:			return "ac_beastfu";
			case ac_axe: 				return "ac_axe";
			case ac_club: 				return "ac_club";
			case ac_dagger: 			return "ac_dagger";
			case ac_hammer: 			return "ac_hammer";
			case ac_mace: 				return "ac_mace";
			case ac_staff: 				return "ac_staff";
			case ac_sword: 				return "ac_sword";
			case ac_bow: 				return "ac_bow";
			case ac_minigun: 			return "ac_minigun";
			case ac_arrow: 				return "ac_arrow";
			case ac_bolt:				return "ac_bolt";
			case ac_combat_magic: 		return "ac_combat_magic";
			case ac_nature_magic: 		return "ac_nature_magic";
			
			default: break;
		}
		return "";
	}

	inline eAttackClass acStringToNum(const std::string& s)
	{
	    if (s == "ac_beastfu") 				return ac_beastfu;
	    else if (s == "ac_axe") 			return ac_axe;
	    else if (s == "ac_club") 			return ac_club;
	    else if (s == "ac_dagger") 			return ac_dagger;
	    else if (s == "ac_hammer") 			return ac_hammer;
	    else if (s == "ac_mace") 			return ac_mace;
	    else if (s == "ac_staff") 			return ac_staff;
	    else if (s == "ac_sword") 			return ac_sword;
	    else if (s == "ac_bow") 			return ac_bow;
	    else if (s == "ac_minigun") 		return ac_minigun;
	    else if (s == "ac_arrow") 			return ac_arrow;
	    else if (s == "ac_bolt") 			return ac_bolt;
	    else if (s == "ac_combat_magic") 	return ac_combat_magic;
	    else if (s == "ac_nature_magic") 	return ac_nature_magic;

	    return ac_invalid; // unknown string
	}


#endif /* EATTACKCLASS_HPP_ */
