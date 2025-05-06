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

#ifndef INGAME_HPP_
#define INGAME_HPP_

	#include "net/Connection.hpp"
	#include "net/packets.hpp"
	
	#include "Engine.hpp"
	#include "msg/WorldMessage.hpp"
	#include "events/SendWorldMessageEvent.hpp"
	
	#include "map/WorldMap.hpp"
	
	class InGame : public WorldState
	{
		public:

		const char* GetPacketName(u_int8_t type, bool isServerToClient) {
		    switch (isServerToClient) {
		    case true: {
		    	switch (type) {
		       // if (isServerToClient == true) {
		        case 1: return "RSAUTHENTICATION";
		        case 2: return "RSPLAYERREADY";
		        case 11: return "RSCHAT";
		        case 12: return "RSMOVE";
		        case 20: return "RSGET";
		        case 21: return "RSDROP";
		        case 22: return "RSEQUIP";
		        case 23: return "RSUNEQUIP";
		        case 24: return "RSATTACKMELEE";
		        case 25: return "RSATTACKRANGED";
		        case 26: return "RSCAST";
		        case 40: return "RSJOBTRAVELDISTANCEREACHED";

		        default: return "UNKNOWN";
		    	}
			}
		    default: {
		    	switch (type) {
		    	case 1: return "RCAUTHENTICATION";
		    	    case 6: return "RCCREATEACTOR";
		    	    case 7: return "RCCREATEITEM";
		    	    case 9: return "RCDISPLAYMESSAGE";
		    	    case 10: return "RCCREATEGO";
		    	    case 11: return "RCCHAT";
		    	    case 12: return "RCMOVE";
		    	    case 13: return "RCDESTROYGO";
		    	    case 14: return "RCSETSCREENHERO";
		    	    case 20: return "RCGET";
		    	    case 21: return "RCDROP";
		    	    case 22: return "RCEQUIP";
		    	    case 23: return "RCUNEQUIP";
		    	    case 24: return "RCATTACKMELEE";
		    	    case 25: return "RCATTACKRANGED";
		    	    case 27: return "RCAPPROACH";

		    	    default: return "UNKNOWN";
		    	}
		    }
		    }
		}

			InGame (Connection & connection, Go * go) : WorldState (&connection), m_go (go)
			{
			}
			
			~InGame ()
			{
			}
			
			void OnEnter ()
			{
				cout << m_go->Goid() << " entered the game" << endl;
				
				m_go->Placement()->SetPosition (SiegePos (0xf1956594, 0.0f, 0.0f, 0.0f));
			}
			
			void OnExit ()
			{
				cout << m_go->Goid() << " left the game" << endl;
				
				m_go->RemoveComponent ("player");
				
				SendWorldMessage (we_left_world, m_go, m_go, "");
			}
			
			void Handle (const char * buffer)
			{
				Packet incoming (buffer);
				
				u_int8_t type = incoming.ReadUInt8();

		        std::cout << "[HANDLE SEND] Type: " << GetPacketName(type, true)
						  << " (id=" << (int)type << ")" << std::endl;

				switch (type)
				{
					case RSPLAYERREADY:
					{
						SendWorldMessage (we_entered_world, m_go, m_go, "1");
						m_go->Send (WorldMessage (we_player_changed, m_go, m_go, ""));
					}
					break;
					
					case RSMOVE:
					{
						u_int32_t node = incoming.ReadUInt32();
						float x = incoming.ReadFloat();
						float y = incoming.ReadFloat();
						float z = incoming.ReadFloat();
						
						m_go->Mind()->Move (SiegePos (node, x, y, z));
					}
					break;
					
					case RSCHAT:
					{
						string message = incoming.ReadString();
						SendWorldMessage (we_req_talk, m_go, m_go, message); 
					}
					break;
					
					case RSGET:
					{
						u_int32_t id = incoming.ReadUInt32();
						
						Go * item = godb.FindGoById (id);
						if (item != NULL)
						{
							//cout << "rsget called on " << item.GetGo()->Common()->Name() << " at " << CurrentTime() << endl;
							m_go->Mind()->Get (item);
						}
					}
					break;
					
					case RSDROP:
					{
						u_int32_t id = incoming.ReadUInt32();
						
						Go * item = godb.FindGoById (id);
						if (item != NULL)
						{
							cout << "dropping item " << id << endl;
							m_go->Mind()->Drop (item);
						}
					}
					break;
					
					case RSEQUIP:
					{
						u_int32_t id = incoming.ReadUInt32();
						u_int8_t slot = incoming.ReadUInt8();
						
						Go * item = godb.FindGoById (id);
						if (item != NULL)
						{
							cout << "equipping " << id << " to slot " << ToString((eEquipSlot)slot) << endl;
							m_go->Mind()->Equip ((eEquipSlot) slot, item);
						}
					}
					break;
					
					case RSUNEQUIP:
					{
						u_int8_t slot = incoming.ReadUInt8();
						
						Go * item = m_go->Inventory()->GetEquipped ((eEquipSlot) slot);
						if (item != NULL)
						{
							m_go->Mind()->Unequip ((eEquipSlot) slot);
						}
					}
					break;
					
					case RSATTACKMELEE:
					{
						u_int32_t id = incoming.ReadUInt32();
						
						Go * target = godb.FindGoById (id);
						if (target != NULL)
						{
							if (m_go->Goid() != id)
							{
								m_go->Mind()->AttackMelee (target);
							}
						}
					}
					break;
					
					case RSATTACKRANGED:
					{
						u_int32_t id = incoming.ReadUInt32();
						
						Go * target = godb.FindGoById (id);
						if (target != NULL)
						{
							if (m_go->Goid() != id)
							{
								cout << "attacking " << id << endl;
								m_go->Mind()->AttackRanged (target);
							}
						}
					}
					break;
					
					case RSINVENMOVE:
					{
						std::cout << "RsInvenmove" << std::endl;
						u_int32_t id = incoming.ReadUInt32();
						u_int8_t origLoc = incoming.ReadUInt8();
						u_int8_t destLoc = incoming.ReadUInt8();

						Go * item = godb.FindGoById (id);
						if (item != NULL)
						{
							cout << "moving " << id << " from slot " << (eInventoryLocation)origLoc << " to " << (eInventoryLocation)destLoc << endl;
							if (destLoc == il_main)
							{
								std::cout << "RsInvenmove2" << std::endl;
								//delete item from location llist
								//GopSet::const_iterator iterator = m_go->Inventory()->m_bag.find ((Go *)item);
								//m_go->Inventory()->m_bag.erase(iterator);
								m_go->Inventory()->RemoveFromSpellbook((eInventoryLocation)origLoc);
								/*map<eInventoryLocation, Go *>::iterator iterator = m_go->Inventory()->m_bag.find ((eInventoryLocation)origLoc);
								if (iterator != m_go->Inventory()->m_bag.end())
								{
									std::cout << "RsInvenmove3" << std::endl;
									m_go->Inventory()->m_bag.erase (iterator);
								}*/
							}
							else
							{
								std::cout << "RsInvenmove4" << std::endl;
								m_go->Inventory()->SetInventoryLocation(item, (eInventoryLocation)destLoc);
								//m_go->Inventory()->m_bag[(eInventoryLocation)destLoc] = item;
							}
						}
					}
					break;

					case RSJOBTRAVELDISTANCEREACHED:
					{
						u_int32_t node = incoming.ReadUInt32();
						float x = incoming.ReadFloat();
						float y = incoming.ReadFloat();
						float z = incoming.ReadFloat();
						
						//cout << "rsjobtraveldistancereached called at " << CurrentTime() << endl;
						m_go->Placement()->SetPosition (SiegePos (node, x, y, z));
						m_go->Mind()->ContinueLastAction();
					}
					break;
					
					default: break;
				}
			}
			
		private:
			
			Go * m_go;
	};

#endif /* INGAME_HPP_ */
