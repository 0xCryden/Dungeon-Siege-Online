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
#include "helper/Log.h"
#include "map/WorldMap.h"

class InGame : public WorldState
{
	public:

	const char* GetPacketName(uint8_t type, bool isServerToClient) {
		switch (isServerToClient) {
		case true: {
		    switch (type) {
		    // if (isServerToClient == true) {
		    case 1: return "RSAUTHENTICATION";
		    case 2: return "RSPLAYERREADY";
	    	case 6: return "RSCREATEACTOR";
	    	case 7: return "RSCREATEITEM";
	    	case 9: return "RSDISPLAYMESSAGE";
		    case 11: return "RSCHAT";
		    case 12: return "RSMOVE";
		    case 20: return "RSGET";
		    case 21: return "RSDROP";
		    case 22: return "RSEQUIP";
		    case 23: return "RSUNEQUIP";
		    case 24: return "RSATTACKMELEE";
		    case 25: return "RSATTACKRANGED";
		    case 26: return "RSCAST";
		    case 27: return "RSAPPROACH";
		    case 28: return "RSKILL";
		    case 29: return "RSDAMAGE";
	    	case 31: return "RSINVENMOVE";
	    	case 32: return "RSREQNODEINFO";
		    case 40: return "RSJOBTRAVELDISTANCEREACHED";

		    default: return "UNKNOWN";
		    }
		}
		default: {
		    switch (type) {
		    	case 1: return "RCAUTHENTICATION";
		        case 2: return "RCPLAYERREADY";
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
			    case 28: return "RCKILL";
			    case 29: return "RCDAMAGE";
		    	case 31: return "RCINVENMOVE";
		    	case 32: return "RCREQNODEINFO";

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
			//SiegePos pos = m_go->Placement()->Position();

				/*if (!pos.IsValid())
				{*/
			Log::Write(Log::Level::INFO, to_string(m_go->Goid()) + " entered the game");

			/*auto now = std::chrono::system_clock::now();
			std::time_t now_time = std::chrono::system_clock::to_time_t(now);
			std::tm local_tm{};

#ifdef _WIN32
			localtime_s(&local_tm, &now_time); // Windows
#else
			localtime_r(&now_time, &local_tm); // POSIX (Linux, macOS)
#endif
			std::cout << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] "
				<< m_go->Goid() << " entered the game" << std::endl;*/

		    	    // Use fallback spawn with randomness
    	            /* std::random_device rd;
    	            std::mt19937 gen(rd());
    	            std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    	            float randX = dist(gen);
    	            float randZ = dist(gen);

    	            float baseX = -2.7f;
    	            float baseZ = -0.8f;

    	            SiegePos fallbackPos(0x4ee0a82e, baseX + randX, 0.0f, baseZ + randZ);
    	            m_go->Placement()->SetPosition(fallbackPos);*/
    	            //0x536d99a7 farmhouse
    	            //0x4ee0a82e multiplayer_world town center
    	            // 0xf1956594 dsmmorpg map
					m_go->Placement()->SetPosition (SiegePos (0x4ee0a82e, 0.0f, 0.0f, 0.0f));
				/*}
				else
				{
				    m_go->Placement()->SetPosition(pos);
				}*/
		}
			
		void OnExit ()
		{
			cout << m_go->Goid() << " left the game" << endl;
			m_go->SaveToXml("actors");
			m_go->RemoveComponent ("player");
				
			SendWorldMessage (we_left_world, m_go, m_go, "");
		}
			
		void Handle (const char * buffer)
		{
			Packet incoming (buffer);
				
			uint8_t type = incoming.ReadUInt8();

    		/*auto now = std::chrono::system_clock::now();
    		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    	    std::cout << "[" << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") << "] "
    	        		<< "[RECV INGAME] Type: " << GetPacketName(type, true)
						<< " (id=" << (int)type << ")" << std::endl;*/

			// this is receive!
			switch (type)
			{
				case RSPLAYERREADY:
				{
					SendWorldMessage (we_entered_world, m_go, m_go, "town_center"); //start region
					m_go->Send (WorldMessage (we_player_changed, m_go, m_go, ""));
				}
				break;
					
				case RSMOVE:
				{
					uint32_t node = incoming.ReadUInt32();
					float x = incoming.ReadFloat();
					float y = incoming.ReadFloat();
					float z = incoming.ReadFloat();
						
					string region = incoming.ReadString();
					string worldname = incoming.ReadString();
					cout << "RSMOVE: Player " << m_go->Common()->ScreenName() <<  " | Node " << node << " | x: " << x << " y: " << y << " z: " << z << endl;

					m_go->Mind()->Move (SiegePos (node, x, y, z));
				}
				break;
					
				case RSCHAT:
				{
					string message = incoming.ReadString();
					if (!message.empty() && message[0] == '/')
					{
						if (m_go->Admin() < 1)
							return;
						// Handle as a command
						m_go->HandleCommand(message);
					}
					else
						SendWorldMessage (we_req_talk, m_go, m_go, message);
				}
				break;
					
				case RSGET:
				{
					uint32_t id = incoming.ReadUInt32();
						
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
					uint32_t id = incoming.ReadUInt32();
						
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
					uint32_t id = incoming.ReadUInt32();
					uint8_t slot = incoming.ReadUInt8();
						
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
					uint8_t slot = incoming.ReadUInt8();
						
					Go * item = m_go->Inventory()->GetEquipped ((eEquipSlot) slot);
					if (item != NULL)
					{
						m_go->Mind()->Unequip ((eEquipSlot) slot);
					}
				}
				break;
					
				case RSATTACKMELEE:
				{
					uint32_t id = incoming.ReadUInt32();
					cout << m_go->Common()->ScreenName() << " wants to attack " << id << endl;
					if (m_go->Mind()->ActionObject() && (id == m_go->Mind()->ActionObject()->Goid()))
						return;

					Go * target = godb.FindGoById (id);
					if (target == NULL)
						return;

					if (m_go->Goid() == id)
						return;

					m_go->Mind()->AttackMelee (target);


				}
				break;
					
				case RSATTACKRANGED:
				{
					uint32_t id = incoming.ReadUInt32();
					cout << m_go->Common()->ScreenName() << " wants to attack " << id << endl;
					if (m_go->Mind()->ActionObject() && (id == m_go->Mind()->ActionObject()->Goid()))
						return;
						
					Go * target = godb.FindGoById (id);
					if (target == NULL)
						return;

					if (m_go->Goid() == id)
						return;

					m_go->Mind()->AttackRanged (target);


				}
				break;
					
				case RSINVENMOVE:
				{
					std::cout << "[RSINVENMOVE] Received inventory move command" << std::endl;

					uint32_t id = incoming.ReadUInt32();
					uint32_t containerId = incoming.ReadUInt32();
					uint8_t location = incoming.ReadUInt8();

					std::cout << "[RSINVENMOVE] Parsed - Item ID: " << id
					            << ", Container ID: " << containerId
					            << ", Location: " << static_cast<int>(location) << std::endl;

					Go* item = godb.FindGoById(id);
					if (item == nullptr)
					{
					    std::cerr << "[RSINVENMOVE] ERROR: Item " << id << " not found in GoDb" << std::endl;
					    return;
					}

					if (containerId == item->Parent()->Goid())
					{
					    std::cerr << "[RSINVENMOVE] ERROR: new container is old container " << containerId << std::endl;
					    return;
					}

					std::cout << "[RSINVENMOVE] Found item Goid: " << item->Goid() << std::endl;

					if (containerId == 0)
					{
					    containerId = m_go->Goid();
					    std::cout << "[RSINVENMOVE] containerId was 0, defaulting to m_go->Goid(): " << containerId << std::endl;
					}

					Go* container = godb.FindGoById(containerId);
					if (container == nullptr)
					{
					    std::cerr << "[RSINVENMOVE] ERROR: Container " << containerId << " not found in GoDb" << std::endl;
					    return;
					}
					std::cout << "[RSINVENMOVE] Found container Goid: " << container->Goid() << std::endl;

					// Transfer item
					std::cout << "[RSINVENMOVE] Initiating transfer..." << std::endl;
					m_go->Inventory()->Transfer(item, container, static_cast<eInventoryLocation>(location));
					std::cout << "[RSINVENMOVE] Transfer completed successfully" << std::endl;

					// Set selected slot if spell slot
					if (location == il_spell_1)
					{
					    std::cout << "[RSINVENMOVE] Location is il_spell_1, setting selected slot to 3" << std::endl;
					    m_go->Inventory()->SetSelectedSlot(3);
					}
					else if (location == il_spell_2)
					{
					    std::cout << "[RSINVENMOVE] Location is il_spell_2, setting selected slot to 4" << std::endl;
					    m_go->Inventory()->SetSelectedSlot(4);
					}

					std::cout << "[RSINVENMOVE] Inventory move completed" << std::endl;
				}
				break;


				case RSREQNODEINFO:
				{

					uint32_t id = incoming.ReadUInt32();
					float x  = incoming.ReadFloat();
					float y = incoming.ReadFloat();
					float z = incoming.ReadFloat();
					uint8_t rotation = incoming.ReadUInt8();

					//new position for fixing up old unknown pos
					uint32_t pid = incoming.ReadUInt32();
					float px  = incoming.ReadFloat();
					float py = incoming.ReadFloat();
					float pz = incoming.ReadFloat();

					Region * region = g_world.GetRegion (m_go->Placement()->GetRegion());
					//Region * region = world.GetRegion (regionname);
					region->AddNode(id, x, y, z, rotation);
					m_go->SetWaitForNodeInfo(false);

					m_go->Placement()->SetPosition(SiegePos(pid, px, py, pz));
				}
				break;

				case RSSELECTSLOT:
				{
					uint8_t selectedSlot = incoming.ReadUInt8();

					GoInventory * inv = m_go->Inventory();
					//if ((uint8_t)selectedSlot != inv->GetSelectedSlot())


					inv->SetSelectedSlot((int)selectedSlot);
					//cout << "[RSSELECTSLOT] User selected: " << (int)selectedSlot << endl;
				}
				break;

				case RSJOBTRAVELDISTANCEREACHED:
				{
					uint32_t node = incoming.ReadUInt32();
					float x = incoming.ReadFloat();
					float y = incoming.ReadFloat();
					float z = incoming.ReadFloat();
						
					//cout << "rsjobtraveldistancereached called at " << CurrentTime() << endl;
					m_go->Placement()->SetPosition (SiegePos (node, x, y, z));
					m_go->Mind()->ContinueLastAction();

					/*auto now = std::chrono::system_clock::now();
					std::time_t now_time = std::chrono::system_clock::to_time_t(now);
					std::cout << "[" << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") << "] "
							<< m_go->Common()->ScreenName() << " moved to "
							<< std::fixed << std::setprecision(1)
							<< m_go->Placement()->WorldPosition().x << "/"
							<< m_go->Placement()->WorldPosition().y << "/"
							<< m_go->Placement()->WorldPosition().z <<	std::endl;*/
				}
				break;

				default: break;
			}
		}
			
	private:
			
		Go * m_go;
};

#endif /* INGAME_HPP_ */
