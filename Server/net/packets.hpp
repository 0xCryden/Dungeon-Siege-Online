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

#ifndef PACKETS_HPP_
#define PACKETS_HPP_

//####### SEND ##########################
	#define RSAUTHENTICATION 1
	#define RSPLAYERREADY 2
	#define RSCREATEACTOR 6
	#define RSCREATEITEM 7
	#define RSDISPLAYMESSAGE 9
	#define RSCREATEGO 10
	#define RSCHAT 11
	#define RSMOVE 12
	#define RSDESTROYGO 13
	#define RSSETSCREENHERO 14
	#define RSGET 20
	#define RSDROP 21
	#define RSEQUIP 22
	#define RSUNEQUIP 23
	#define RSATTACKMELEE 24
	#define RSATTACKRANGED 25
	#define RSCAST 26
	#define RSAPPROACH 27
	#define RSINVENMOVE 31
	#define RSREQNODEINFO 32
	#define RSUPDATEGO 33
	#define RSADDEXP 34
	#define RSLEVELUP 35
	#define RSSELECTSLOT 36

	#define RSREGISTER 37
	#define RSREQCHARLIST 38
	#define RSENTERGAME 39
	#define RSJOBTRAVELDISTANCEREACHED 40
	#define RSDELETECHAR 41
	#define RSCREATECHAR 42
//####### RECV ##########################
	#define RCAUTHENTICATION 1

	#define RCCREATEACTOR 6
	#define RCCREATEITEM 7

	#define RCDISPLAYMESSAGE 9
	#define RCCREATEGO 10
	#define RCCHAT 11
	#define RCMOVE 12
	#define RCDESTROYGO 13
	#define RCSETSCREENHERO 14

	#define RCGET 20
	#define RCDROP 21
	#define RCEQUIP 22
	#define RCUNEQUIP 23
	#define RCATTACKMELEE 24
	#define RCATTACKRANGED 25

	#define RCAPPROACH 27

	#define RCINVENMOVE 31
	#define RCREQNODEINFO 32

	#define RCUPDATEGO 33
	#define RCADDEXP 34
	#define RCLEVELUP 35
	#define RCSELECTSLOT 36

	#define RCREGISTER 37
	#define RCREQCHARLIST 38
	#define RCENTERGAME 39
	#define RCDELETECHAR 41
	#define RCCREATECHAR 42

#endif /* PACKETS_HPP_ */
