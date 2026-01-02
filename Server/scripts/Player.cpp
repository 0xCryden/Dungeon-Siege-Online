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

#include "Player.hpp"
#include "../net/packets.hpp"
#include "../net/Connection.hpp"

#include <iostream>
#include <vector>
#include "../Engine.hpp"
using namespace std;

Player :: Player (Connection * connection, Go * go) : GoScriptComponent (go), m_connection (connection)
{
}

void CollectInventoryRecursive(const GopSet& inventorySet,
                               std::vector<Go*>& itemsWithoutInventory,
                               std::vector<Go*>& itemsWithInventory)
{
    for (Go* item : inventorySet)
    {
        if (item->HasInventory())
        {
            itemsWithInventory.push_back(item);
            const GopSet& nestedInventory = item->Inventory()->ListItems();
            CollectInventoryRecursive(nestedInventory, itemsWithoutInventory, itemsWithInventory);
        }
        else
        {
            itemsWithoutInventory.push_back(item);
        }
    }
}

// Helper function: writes an item and its contained items recursively
void WriteItemRecursive(Go* item, Packet& packet)
{
	if (!item)
		return;

	Go* parent = item->Parent();
	GoInventory* parentInv = parent ? parent->Inventory() : nullptr;

	uint32_t containerId = parent ? parent->Goid() : 0;

	eEquipSlot slot = (parent && parent->Inventory())
	        ? parent->Inventory()->GetEquippedSlot(item)
	        : es_none;

    eInventoryLocation loc = il_main;
    if (parent && parentInv)
        loc = item->GetLoc();//parentInv->GetInventoryLocation(item);

    /*std::cout << "Sending item: ID=" << item->Goid()
                  << ", Slot=" << slot
                  << ", Loc=" << ToString(loc)
                  << ", ContainerID=" << containerId << std::endl;*/

    packet.WriteUInt32(item->Goid());
    packet.WriteString(item->Common()->ScreenName());
    packet.WriteUInt8(slot);
    packet.WriteUInt8(loc);
    packet.WriteUInt32(containerId);
	packet.WriteString(item->TemplateName());
	packet.WriteString(item->pContentQuery());

    //GoInventory* inv = item->Inventory();
    if (item->HasInventory() && !item->Inventory()->ListItems().empty())
    {
    	GoInventory* inv = item->Inventory();
        packet.WriteUInt8(1); // has inventory

        const GopSet& inveen = inv->ListItems();
        packet.WriteUInt8(inveen.size());
        for (Go* child : inv->ListItems())
        {
            WriteItemRecursive(child, packet);
        }
    }
    else
    {
        packet.WriteUInt8(0); // no inventory
    }
}

void SendRCCreateItemRecursive(Go* item, Connection* conn)
{
	if (!item || !item->IsItem())
		return;

	Packet packet;
	packet.WriteUInt8(RCCREATEITEM);
	packet.WriteUInt32(item->Goid());
	packet.WriteString(item->Common()->ScreenName());
	packet.WriteString(item->TemplateName());
	packet.WriteString(item->pContentQuery());
	packet.WriteUInt32(item->Placement()->Position().Node);
	packet.WriteFloat(item->Placement()->Position().X);
	packet.WriteFloat(item->Placement()->Position().Y);
	packet.WriteFloat(item->Placement()->Position().Z);

	cout << "RCCREATEITEM pContentQuery: " << item->pContentQuery() << endl;
	// Write nested inventory
	/*if (item->HasInventory() && !item->Inventory()->ListItems().empty())
	{
		GoInventory* inv = item->Inventory();
		packet.WriteUInt8(1); // has inventory
		packet.WriteUInt8(inv->ListItems().size()); // how many inside
		for (Go* child : inv->ListItems())
		{
			cout << "SendRCCreateItemRecursive ID: " << child->Goid() << endl;
			// Recursively write child items
			WriteItemRecursive(child, packet);
		}
	}
	else*/
	{
		packet.WriteUInt8(0); // no inventory
	}

	//cout << "Sending RCCreateItem: ID=" << item->Goid() << std::endl;
	conn->Send(packet.Data(), packet.Size());
}

void Player :: OnGoHandleMessage (const WorldMessage & message)
{
	eWorldEvent event = message.WorldEvent();
	Go * from = message.SendFrom();
	Go * to = message.SendTo();
	
	if (event != we_entered_frustum && event != we_left_frustum &&
		(!(event == we_mind_processing_new_job && message.Data() == "jat_move") &&
		(event != we_unknown_node)))
		cout << "received event " << ToString (message.WorldEvent()) << " from " << from->Goid() << " to " << to->Goid() << endl;

	switch (event)
	{
		case we_goupdate_lifestate:
		{
			if (from->HasCommon())
			{
				cout << "Sending go update for " << from->Common()->ScreenName() << endl;

				Packet packet;
				packet.WriteUInt8(RCUPDATEGO);
				packet.WriteUInt8(we_goupdate_lifestate);
				packet.WriteUInt32(from->Goid());

				packet.WriteFloat(from->Aspect()->CurrentLife());
				packet.WriteFloat(from->Aspect()->MaxLife());
				packet.WriteFloat(from->Aspect()->CurrentMana());
				packet.WriteFloat(from->Aspect()->MaxMana());
				packet.WriteUInt8 (from->Aspect()->LifeState());

				m_connection->Send(packet.Data(), packet.Size());
			}
		}
		break;

		case we_goupdate_skills:
		{
			if (from->HasCommon())
			{
				cout << "Sending go update for " << from->Common()->ScreenName() << endl;

				Packet packet;
				packet.WriteUInt8(RCUPDATEGO);
				packet.WriteUInt8(we_goupdate_skills);
				packet.WriteUInt32(from->Goid());

				packet.WriteFloat(from->Actor()->GetSkillLevel("uber"));
				packet.WriteFloat(from->Actor()->GetSkillLevel("strength"));
				packet.WriteFloat(from->Actor()->GetSkillLevel("intelligence"));
				packet.WriteFloat(from->Actor()->GetSkillLevel("dexterity"));
				packet.WriteFloat(from->Actor()->GetSkillLevel("melee"));
				packet.WriteFloat(from->Actor()->GetSkillLevel("ranged"));
				packet.WriteFloat(from->Actor()->GetSkillLevel("nature magic"));
				packet.WriteFloat(from->Actor()->GetSkillLevel("combat magic"));

				m_connection->Send(packet.Data(), packet.Size());
			}
		}
		break;

		case we_unknown_node:
		{
			if (from->HasCommon())
			{
				Packet packet;
				packet.WriteUInt8 (RCREQNODEINFO);
				packet.WriteUInt32 (from->GetLastPos().Node);
				packet.WriteFloat (from->GetLastLocal().x);
				packet.WriteFloat (from->GetLastLocal().y);
				packet.WriteFloat (from->GetLastLocal().z);
				packet.WriteUInt8 (from->GetLastRota());
				packet.WriteUInt32 (from->Placement()->Position().Node);

				//cout << "Sending request Lastlocal: " << from->GetLastLocal().x << "/" << from->GetLastLocal().y << "/" << from->GetLastLocal().z << endl;

				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;

		case we_add_exp:
		{
			if (from->HasCommon())
			{
				Packet packet;
				packet.WriteUInt8 (RCADDEXP);
				packet.WriteUInt32 (from->Goid());

				float amount = 0.0;
				try {
				    amount = std::stof(message.Data());
				} catch (const std::exception& e) {
					cout << "Error applying amount: " << e.what() << endl;
				    // handle error
				}

				packet.WriteFloat (amount);

				//cout << "Sending RCADDEXP amount: " << amount << endl;

				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;

		case we_leveled_up:
		{
			if (from->HasCommon())
			{
				Packet packet;
				packet.WriteUInt8 (RCLEVELUP);
				packet.WriteUInt32 (from->Goid());
				packet.WriteString (message.Data());

				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;

		case we_entered_world:
		{
			if (from->HasCommon())
			{
				string message = from->Common()->ScreenName() + " has entered the world";
				
				Packet packet;
				packet.WriteUInt8 (RCDISPLAYMESSAGE);
				packet.WriteString (message);
				
				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;
		
		case we_left_world:
		{
			if (from->HasCommon())
			{
				string message = from->Common()->ScreenName() + " has left the world";
				
				Packet packet;
				packet.WriteUInt8 (RCDISPLAYMESSAGE);
				packet.WriteString (message);
				
				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;
		
		case we_entered_frustum:
		{
			if (from->IsActor()/* && from->Conversations().empty()*/)
			{
				Packet packet;
				packet.WriteUInt8 (RCCREATEACTOR);
				packet.WriteUInt32(from->Goid());
				packet.WriteUInt32(from->Scid());
				packet.WriteString (from->Common()->ScreenName());
				packet.WriteString (from->TemplateName());
				packet.WriteString(from->Aspect()->Model());
				packet.WriteUInt8 (from->Actor()->Alignment());
				packet.WriteFloat (from->Aspect()->RenderScale());
				packet.WriteString (from->Aspect()->GetDynamicTexture (0));
				packet.WriteString (from->Aspect()->GetDynamicTexture (1));
				if (from->HasInventory())
					packet.WriteString (from->Inventory()->GetCustomHead());
				else
					packet.WriteString ("");
				packet.WriteUInt32 (from->Placement()->Position().Node);
				packet.WriteFloat (from->Placement()->Position().X);
				packet.WriteFloat (from->Placement()->Position().Y);
				packet.WriteFloat (from->Placement()->Position().Z);
				
				// life
				packet.WriteFloat (from->Aspect()->MaxLife());
				packet.WriteFloat (from->Aspect()->CurrentLife());
				packet.WriteFloat (from->Aspect()->MaxMana());
				packet.WriteFloat (from->Aspect()->CurrentMana());
				packet.WriteUInt8 (from->Aspect()->LifeState());
				// skills
				packet.WriteFloat (from->Actor()->GetSkillLevel("uber"));
				packet.WriteFloat (from->Actor()->GetSkillLevel("strength"));
				packet.WriteFloat (from->Actor()->GetSkillLevel("intelligence"));
				packet.WriteFloat (from->Actor()->GetSkillLevel("dexterity"));
				packet.WriteFloat (from->Actor()->GetSkillLevel("melee"));
				packet.WriteFloat (from->Actor()->GetSkillLevel("ranged"));
				packet.WriteFloat (from->Actor()->GetSkillLevel("nature magic"));
				packet.WriteFloat (from->Actor()->GetSkillLevel("combat magic"));

				for (int i = 0; i < 12; i++)
				{
					Go * equipment = from->Inventory()->GetEquipped ((eEquipSlot) i);
					
					if (equipment != NULL)
					{
						packet.WriteUInt32 (equipment->Goid());
						packet.WriteString(equipment->TemplateName());
						packet.WriteString(equipment->pContentQuery());
					}
					else
					{
						packet.WriteUInt32 (0);
						packet.WriteString("");
						packet.WriteString("");
					}
				}

				m_connection->Send (packet.Data(), packet.Size());

				return;
			}
			
			if (from->IsItem())
			{
				SendRCCreateItemRecursive(from, m_connection);
				return;
			}

		}
		break;
		
		case we_left_frustum:
		{
			if (from != NULL)
			{
				Packet packet;
				packet.WriteUInt8 (RCDESTROYGO);
				packet.WriteUInt32 (from->Goid());
				
				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;
		
		case we_player_changed:
		{
			if (to->IsActor())
			{
				Packet packet;
				packet.WriteUInt8 (RCSETSCREENHERO);
				packet.WriteUInt32 (to->Goid());

				// life
				packet.WriteFloat (to->Aspect()->MaxLife());
				packet.WriteFloat (to->Aspect()->CurrentLife());
				packet.WriteFloat (to->Aspect()->MaxMana());
				packet.WriteFloat (to->Aspect()->CurrentMana());
				packet.WriteUInt8 (to->Aspect()->LifeState());
				// stats
				packet.WriteFloat (to->Actor()->GetSkillLevel("uber"));
				packet.WriteFloat (to->Actor()->GetSkillLevel("strength"));
				packet.WriteFloat (to->Actor()->GetSkillLevel("intelligence"));
				packet.WriteFloat (to->Actor()->GetSkillLevel("dexterity"));
				packet.WriteFloat (to->Actor()->GetSkillLevel("melee"));
				packet.WriteFloat (to->Actor()->GetSkillLevel("ranged"));
				packet.WriteFloat (to->Actor()->GetSkillLevel("nature magic"));
				packet.WriteFloat (to->Actor()->GetSkillLevel("combat magic"));

				// Get top-level inventory items
				const GopSet& inventory = to->Inventory()->ListItems();

				// Write number of top-level items (not counting nested ones)
				packet.WriteUInt8(inventory.size());

				for (Go* item : inventory)
				{
				    WriteItemRecursive(item, packet);
				}

				
				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;
		
		case we_req_talk:
		{
			if (from != NULL)
			{
				Packet packet;
				if (to->HasCommon())
				{
					//string msg = "<" + from->Common()->ScreenName() + "> : " + message.Data();
					packet.WriteUInt8 (RCCHAT);
					packet.WriteUInt32 (from->Goid());
					packet.WriteString (from->Common()->ScreenName());
					packet.WriteString (message.Data());
				}
				
				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;
		
		case we_picked_up:
		{
			if (to->IsItem())
			{
				Packet packet;
				packet.WriteUInt8 (RCGET);
				packet.WriteUInt32 (to->Goid());
				packet.WriteString (to->Common()->ScreenName());
				packet.WriteString (to->TemplateName());
				packet.WriteString(to->pContentQuery());

				if (to->HasInventory() && !to->Inventory()->ListItems().empty())
			    {
					packet.WriteUInt8(1); // has inventory
			        const GopSet& inveen = to->Inventory()->ListItems();
			        packet.WriteUInt8(inveen.size());

					for (Go* child : to->Inventory()->ListItems())
			        {
						eEquipSlot slot = es_none;
					    eInventoryLocation loc = il_main;

					    loc = child->GetLoc();//parentInv->GetInventoryLocation(child);

					    std::cout << "Sending item pickup: ID=" << child->Goid()
					                  << ", Slot=" << ToString(slot)
					                  << ", Loc=" << ToString(loc)
					                  << ", ContainerID=" << to->Goid() << std::endl;


					    packet.WriteUInt32(child->Goid());
					    packet.WriteString(child->Common()->ScreenName());
					    packet.WriteUInt8(slot);
					    packet.WriteUInt8(loc);
					    packet.WriteString(child->TemplateName());
						packet.WriteString(child->pContentQuery());
			        }
			    }
			    else
			    {
			        packet.WriteUInt8(0); // no inventory
			    }

				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;
		
		case we_dropped:
		{
			if (to->IsItem())
			{
				Packet packet;
				packet.WriteUInt8 (RCDROP);
				packet.WriteUInt32 (to->Goid());

				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;
		
		case we_engaged_hit_lived:
		{
			//cout << "player heard we_engaged_hit" << endl;
			
			eJobAbstractType jat = from->Mind()->ActionJat();
			if (jat == jat_attack_object_melee)
			{
				Packet packet;
				packet.WriteUInt8 (RCATTACKMELEE);
				packet.WriteUInt32 (from->Goid());
				packet.WriteUInt32 (to->Goid());
				packet.WriteUInt8 (1);
				
				m_connection->Send (packet.Data(), packet.Size());
			}
			
			else if (jat == jat_attack_object_ranged)
			{
				Packet packet;
				packet.WriteUInt8 (RCATTACKRANGED);
				packet.WriteUInt32 (from->Goid());
				packet.WriteUInt32 (to->Goid());
				packet.WriteUInt8 (1);
				
				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;
		
		case we_engaged_missed:
		{
			//cout << "player heard we_engaged_missed" << endl;
			
			eJobAbstractType jat = from->Mind()->ActionJat();
			if (jat == jat_attack_object_melee)
			{
				Packet packet;
				packet.WriteUInt8 (RCATTACKMELEE);
				packet.WriteUInt32 (from->Goid());
				packet.WriteUInt32 (to->Goid());
				packet.WriteUInt8 (2);
				
				m_connection->Send (packet.Data(), packet.Size());
			}
			
			else if (jat == jat_attack_object_ranged)
			{
				Packet packet;
				packet.WriteUInt8 (RCATTACKRANGED);
				packet.WriteUInt32 (from->Goid());
				packet.WriteUInt32 (to->Goid());
				packet.WriteUInt8 (2);
				
				m_connection->Send (packet.Data(), packet.Size());
			}
		}
		break;
		
		case we_mind_processing_new_job:
		{
			if (from->HasMind())
			{
				string job = message.Data();

				if (job != "jat_move")
					cout << "Job: " << job << endl;

				if (job == "jat_move")
				{
					SiegePos destination = from->Mind()->ActionPosition();
					
					Packet packet;
					packet.WriteUInt8 (RCMOVE);
					packet.WriteUInt32 (from->Goid());
					packet.WriteUInt32 (destination.Node);
					packet.WriteFloat (destination.X);
					packet.WriteFloat (destination.Y);
					packet.WriteFloat (destination.Z);
					
					m_connection->Send (packet.Data(), packet.Size());
				}
				
				else if (job == "jat_approach")
				{
					SiegePos destination = from->Mind()->ActionPosition();
					float distance = from->Mind()->ActionDistance();
					
					Packet packet;
					packet.WriteUInt8 (RCAPPROACH);
					packet.WriteUInt32 (from->Goid());
					packet.WriteUInt32 (destination.Node);
					packet.WriteFloat (destination.X);
					packet.WriteFloat (destination.Y);
					packet.WriteFloat (destination.Z);
					packet.WriteFloat (distance);
					
					m_connection->Send (packet.Data(), packet.Size());
				}
				
				else if (job == "jat_equip")
				{
					eEquipSlot slot = from->Mind()->ActionSlot();
					
					Packet packet;
					packet.WriteUInt8 (RCEQUIP);
					packet.WriteUInt32 (from->Goid());
					packet.WriteUInt8 (slot);
					packet.WriteUInt32 (to->Goid());
					packet.WriteString(to->TemplateName());
					packet.WriteString(to->pContentQuery());
					
					m_connection->Send (packet.Data(), packet.Size());
				}
				
				else if (job == "jat_unequip")
				{
					eEquipSlot slot = from->Mind()->ActionSlot();
					
					Packet packet;
					packet.WriteUInt8 (RCUNEQUIP);
					packet.WriteUInt32 (from->Goid());
					packet.WriteUInt8 (slot);
					packet.WriteUInt8 (from->Mind()->ActionObject()->GetLoc());
					
					m_connection->Send (packet.Data(), packet.Size());
				}
			}
		}
		break;
		
		default: break;
	} /* switch (event) */

}
