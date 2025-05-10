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
        loc = parentInv->GetInventoryLocation(item);

    std::cout << "Sending item: ID=" << item->Goid()
                  << ", Slot=" << slot
                  << ", Loc=" << ToString(loc)
                  << ", ContainerID=" << containerId << std::endl;

    packet.WriteUInt32(item->Goid());
    packet.WriteString(item->Common()->ScreenName());
    packet.WriteUInt8(slot);
    packet.WriteUInt8(loc);
    packet.WriteUInt32(containerId);
    packet.WriteString(item->Aspect()->Model());

    //GoInventory* inv = item->Inventory();
    if (item->HasInventory() && !item->Inventory()->ListItems().empty())
    {
    	GoInventory* inv = item->Inventory();
        packet.WriteUInt8(1); // has inventory^

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
	packet.WriteString(item->Aspect()->Model());
	packet.WriteUInt32(item->Placement()->Position().Node);
	packet.WriteFloat(item->Placement()->Position().X);
	packet.WriteFloat(item->Placement()->Position().Y);
	packet.WriteFloat(item->Placement()->Position().Z);

	// Write nested inventory
	/*if (item->HasInventory() && !item->Inventory()->ListItems().empty())
	{
		GoInventory* inv = item->Inventory();
		packet.WriteUInt8(1); // has inventory
		packet.WriteUInt8(inv->ListItems().size()); // how many inside
		for (Go* child : inv->ListItems())
		{
			// Recursively write child items
			WriteItemRecursive(child, packet);
		}
	}
	else*/
	{
		packet.WriteUInt8(0); // no inventory
	}

	std::cout << "Sending RCCreateItem: ID=" << item->Goid() << std::endl;
	conn->Send(packet.Data(), packet.Size());
}

void Player :: OnGoHandleMessage (const WorldMessage & message)
{
	eWorldEvent event = message.WorldEvent();
	Go * from = message.SendFrom();
	Go * to = message.SendTo();
	
	cout << "received event " << ToString (message.WorldEvent()) << " from " << from->Goid() << " to " << to->Goid() << endl;

	switch (event)
	{
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
			if (from->IsActor())
			{
				Packet packet;
				packet.WriteUInt8 (RCCREATEACTOR);
				packet.WriteUInt32 (from->Goid());
				packet.WriteString (from->Common()->ScreenName());
				packet.WriteString (from->Aspect()->Model());
				packet.WriteUInt8 (from->Actor()->Alignment());
				packet.WriteFloat (from->Aspect()->RenderScale());
				packet.WriteString (from->Aspect()->GetDynamicTexture (0));
				packet.WriteString (from->Aspect()->GetDynamicTexture (1));
				packet.WriteUInt32 (from->Placement()->Position().Node);
				packet.WriteFloat (from->Placement()->Position().X);
				packet.WriteFloat (from->Placement()->Position().Y);
				packet.WriteFloat (from->Placement()->Position().Z);
				
				for (int i = 0; i < 12; i++)
				{
					Go * equipment = from->Inventory()->GetEquipped ((eEquipSlot) i);
					
					if (equipment != NULL)
					{
						packet.WriteUInt32 (equipment->Goid());
						packet.WriteString (equipment->Aspect()->Model());
					}
					else
					{
						packet.WriteUInt32 (0);
						packet.WriteUInt8 (0);
					}
				}

			    //std::cout << "From->IsActor()" << std::endl;

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
				
				// stats
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
					string msg = "<" + from->Common()->ScreenName() + "> : " + message.Data();
					packet.WriteUInt8 (RCDISPLAYMESSAGE);
					packet.WriteString (msg);
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
				packet.WriteString (to->Aspect()->Model());

				if (to->HasInventory() && !to->Inventory()->ListItems().empty())
			    {
					packet.WriteUInt8(1); // has inventory
			        const GopSet& inveen = to->Inventory()->ListItems();
			        packet.WriteUInt8(inveen.size());

					for (Go* child : to->Inventory()->ListItems())
			        {
						Go* parent = child->Parent();
						GoInventory* parentInv = parent ? parent->Inventory() : nullptr;

						eEquipSlot slot = es_none;
					    eInventoryLocation loc = il_main;

					    loc = parentInv->GetInventoryLocation(child);

					    std::cout << "Sending item pickup: ID=" << child->Goid()
					                  << ", Slot=" << slot
					                  << ", Loc=" << ToString(loc)
					                  << ", ContainerID=" << to->Goid() << std::endl;


					    packet.WriteUInt32(child->Goid());
					    packet.WriteString(child->Common()->ScreenName());
					    packet.WriteUInt8(slot);
					    packet.WriteUInt8(loc);
					    packet.WriteString(child->Aspect()->Model());
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
				
				//to->SetOwner(0);

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
					packet.WriteString (to->Aspect()->Model());
					
					m_connection->Send (packet.Data(), packet.Size());
				}
				
				else if (job == "jat_unequip")
				{
					eEquipSlot slot = from->Mind()->ActionSlot();
					
					Packet packet;
					packet.WriteUInt8 (RCUNEQUIP);
					packet.WriteUInt32 (from->Goid());
					packet.WriteUInt8 (slot);
					
					m_connection->Send (packet.Data(), packet.Size());
				}
			}
		}
		break;
		
		default: break;
	} /* switch (event) */

}
