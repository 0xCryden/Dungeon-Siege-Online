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

#include "GoDb.hpp"
#include "GoInventory.hpp"

#include "../map/WorldMap.hpp"
#include "../msg/WorldMessage.hpp"

GoInventory :: GoInventory (Go * go) : GoComponent (go)
{
	/*
	GopSet::iterator iterator = m_go->Inventory()->m_inventory.begin();
	while (iterator != m_go->Inventory()->m_inventory.end())
	{
		Go * item = *iterator;
		
		Go * clone = NULL;
		if (item->TemplateName() != "")
		{
			clone = godb.CloneGo (item->TemplateName());
		}
		else
		{
			clone = godb.CloneGo (*item);
		}
		
		if (clone)
		{
			m_inventory.insert (clone);
			// SendWorldMessage (we_entered_world, copy->Id(), copy->Id(), "");
		}
		
		iterator++;
	}
	*/
}

GoInventory :: GoInventory (Go * go, xmlNode * node) : GoComponent (go)
{
	if (node == NULL)
		return;

	for (xmlNode * current = node->children; current != NULL; current = current->next)
	{
		if (current->type != XML_ELEMENT_NODE) 				continue;
		if (!xmlStrEqual(current->name, BAD_CAST "item")) 	continue;

		u_int32_t id = xml::ReadAttribute<u_int32_t> (current, "id", 0);
		std::string equipSlotStr = xml::ReadAttribute<std::string>(current, "equip_slot", "");
		std::string invenLocStr = xml::ReadAttribute<std::string>(current, "inven_loc", "");

		Go* item = nullptr;

		item = godb.FindGoById(id);
		//Goid g (id);
		// add g to our inventory :)
		if (!item)
		{
			cout << "Failed loading item. ID: " << id << " | OwnerGoId: " << m_go->Goid() << endl;
			continue;
		}

		// Set inventory location
		eInventoryLocation invenLoc = StringToNumLoc(invenLocStr); // You already have this
		if (invenLoc != il_main)
			m_bag[invenLoc] = item;

		// Add to inventory
		if (Add(item))
		{
			// If equip_slot is present, equip it
			if (!equipSlotStr.empty())
			{
				eEquipSlot slot = StringToNum(equipSlotStr); // You need to implement this
				if (slot != es_none)
				{
					Equip(slot, item); // Will only succeed if not already equipped
				}
			}
			/*
			if (item->HasInventory() && item->Inventory() == nullptr) {
				item->m_inventory = new GoInventory(item);
			}
			  std::cout << "[CHAR LOAD] Item HasInventory: " << item->HasInventory() << std::endl;
			// Handle nested inventory inside this item
			if (item->HasInventory() || id == 107)
			{
				for (xmlNode* invNode = current->children; invNode != NULL; invNode = invNode->next)
				{
					if (invNode->type != XML_ELEMENT_NODE) continue;
					if (!xmlStrEqual(invNode->name, BAD_CAST "inventory")) continue;

					for (xmlNode* current2 = invNode->children; current2 != NULL; current2 = current2->next)
					{
						if (current2->type != XML_ELEMENT_NODE) continue;
						if (!xmlStrEqual(current2->name, BAD_CAST "item")) continue;

						u_int32_t id2 = xml::ReadAttribute<u_int32_t>(current2, "id", 0);
						std::string invenLocStr2 = xml::ReadAttribute<std::string>(current2, "inven_loc", "");

						//GoInventory* subInventory = item->Inventory();
						//if (!subInventory)
						//	continue;

						Go* item2 = godb.FindGoById(id2);
						if (!item2)
							continue;

						eInventoryLocation invenLoc2 = StringToNumLoc(invenLocStr2);
						if (invenLoc2 != il_main)
							subInventory->m_bag[invenLoc2] = item2;

						subInventory->Add(item2);
					}
				}
			}*/
		}
	}
}

void GoInventory :: Save (xmlNode* inventoryNode) const
{
	if (!inventoryNode)
			return;

	// Step 1: Remove old <item> nodes
	xmlNode* current = inventoryNode->children;
	while (current != NULL)
	{
		xmlNode* next = current->next;
		if (current->type == XML_ELEMENT_NODE && xmlStrEqual(current->name, BAD_CAST "item"))
		{
			xmlUnlinkNode(current);
			xmlFreeNode(current);
		}
		current = next;
	}

	for (GopSet::const_iterator it = m_inventory.begin(); it != m_inventory.end(); ++it)
	{
		Go* item = *it;
		if (!item)
			continue;

		xmlNode* itemNode = xmlNewChild(inventoryNode, NULL, BAD_CAST "item", NULL);

		uint32_t id = item->Goid();
		if (id != 0)
		{
			xml::SetAttribute(itemNode, "id", id);
		}
		else
		{
			std::string templateName = item->TemplateName();
			if (!templateName.empty())
			{
				xml::SetAttribute(itemNode, "template_name", templateName);
			}
		}

		// Step 3: Add equip_slot if equipped
		eEquipSlot slot = GetEquippedSlot(item);
		//if (slot != es_none)
		{
			std::string slotStr = ToString(slot); // Assumes you have a ToString(eEquipSlot) function
			xml::SetAttribute(itemNode, "equip_slot", slotStr);
		}

		eInventoryLocation loc = GetInventoryLocation(item);
		cout << "Saving Item.. ID: " << item->Goid() << " | loc: " << loc << endl;
		if (loc != il_main && loc != il_invalid)
		{
			std::string slotStr2 = ToString(loc);
			xml::SetAttribute(itemNode, "inven_loc", slotStr2);
		}
		else
		{
			xml::SetAttribute(itemNode, "inven_loc", "il_main");
		}
	}


}

bool GoInventory :: Add (Go * item)
{
	if (item != NULL)
	{
		if (Contains (item) != true)
		{
			/*
			 * check if we can actually add this item
			 */
			
			m_inventory.insert (item);
			
			item->SetParent (m_go);
			m_go->AddChild (item);
			
			return true;
		}
	}
	
	return false;
}

bool GoInventory :: Contains (const Go * item) const
{
	if (item != NULL)
	{
		GopSet::const_iterator iterator = m_inventory.find ((Go *)item);
		return iterator != m_inventory.end();
	}
	
	return false;
}

bool GoInventory :: Equip (eEquipSlot slot, Go * item)
{
	if (IsSlotEquipped (slot) != false)
	{
		return false;
	}
	
	if (IsEquipped (item) != false)
	{
		return false;
	}
	
	if (Contains (item) != true)
	{
		return false;
	}
	
	if (item != NULL)
	{
		/*
		 * check if we can actually equip this item
		 */
		
		m_equipment[slot] = item;
		
		return true;
	}
	
	return false;
}

Go * GoInventory :: GetEquipped (eEquipSlot slot) const
{
	map<eEquipSlot, Go *>::const_iterator iterator = m_equipment.find (slot);
	return iterator != m_equipment.end() ? iterator->second : NULL;
}

eEquipSlot GoInventory :: GetEquippedSlot (const Go * item) const
{
	if  (item != NULL)
	{
		map<eEquipSlot, Go *>::const_iterator iterator = m_equipment.begin();
		while (iterator != m_equipment.end())
		{
			if (iterator->second->Goid() == item->Goid())
			{
				return iterator->first;
			}
			
			iterator++;
		}
	}
	
	return es_none;
}

void GoInventory :: SetInventoryLocation (Go * item, eInventoryLocation loc, int ownerId)
{
	if (!item)
        return;

    Go* owner = godb.FindGoById(ownerId);
    if (!owner) {
    	cout << "no owner to set loc" << endl;
        return;
	}

    // Remove from previous container if needed
    if (item->GetOwner() != 0)
    {
        Go* prevOwner = godb.FindGoById(item->GetOwner());
        if (prevOwner && prevOwner->HasInventory())
        {
            prevOwner->Inventory()->Remove(item);
        }
    }

    // Set new state
    item->SetLoc(loc);
    item->SetOwner(ownerId);
    //item->m_inventoryLocation = loc;
    //item->m_inventoryOwnerId = ownerId;

    // Update in owner's inventory
    owner->Inventory()->Add(item);

    // Optional: Update m_bag only if needed
    if (loc != il_main && loc != il_invalid)
    {
        owner->Inventory()->m_bag[loc] = item;
    }

    std::cout << "[SET ITEM LOC] Item " << item->Goid() << " now at " << loc << " owned by " << ownerId << std::endl;

	/*if (item == NULL)
		return;

	// 1. Get equipped spellbook
	Go * spellBook = godb.FindGoById(ownerId);
	// 2. Add item to spellbook
	if (spellBook == NULL)
			return;

	switch (loc) {
	case il_active_primary_spell:
	case il_active_secondary_spell:
	case il_spell_1:
	case il_spell_2:
	case il_spell_3:
	case il_spell_4:
	case il_spell_5:
	case il_spell_6:
	case il_spell_7:
	case il_spell_8:
	case il_spell_9:
	case il_spell_10:
	case il_spell_11:
	case il_spell_12:
		{
			spellBook->Inventory()->m_bag[loc] = item;
			m_bag[loc] = item;
			std::cout << "[INVENTORYLOCATION] id " << item->Goid() << " loc: " << loc << std::endl;
			spellBook->Inventory()->Add(item);
			item->Parent()->Parent()->Inventory()->Remove(item);
		}
		break;
	case il_invalid:
	case il_main:
		{
			map<eInventoryLocation, Go *>::iterator iterator = m_bag.find ((eInventoryLocation)GetInventoryLocation(item));
			if (iterator != m_bag.end())
			{
				std::cout << "[INVENTORYLOCATION] id " << item->Goid() << " loc: " << loc << std::endl;
				m_bag.erase (iterator);
			}
			std::cout << "[INVENTORYLOCATION] 2 id " << item->Goid() << " loc: " << loc << std::endl;
			//spellBook->Inventory()->Remove(item);
			item->Parent()->Inventory()->Remove(item);
			spellBook->Inventory()->Add(item);
		}
		break;
	default:
		break;
	}*/
}

eInventoryLocation GoInventory :: GetInventoryLocation (const Go * item) const
{
	if  (item != NULL)
	{
		map<eInventoryLocation, Go *>::const_iterator iterator = m_bag.begin();
		while (iterator != m_bag.end())
		{
			if (iterator->second->Goid() == item->Goid())
			{
				return iterator->first;
			}

			iterator++;
		}
	}

	return il_main;
}

bool GoInventory :: IsAnyWeaponEquipped () const
{
	Go * item;
	
	item = GetEquipped (es_shield_hand);
	if (item != NULL)
	{
		if (item->IsWeapon())
		{
			return true;
		}
	}
	
	item = GetEquipped (es_shield_hand);
	if (item != NULL)
	{
		if (item->IsWeapon())
		{
			return true;
		}
	}
	
	return false;
}

bool GoInventory :: IsEquipped (const Go * item) const
{
	if  (item != NULL)
	{
		map<eEquipSlot, Go *>::const_iterator iterator = m_equipment.begin();
		while (iterator != m_equipment.end())
		{
			if (iterator->second->Goid() == item->Goid())
			{
				return true;
			}
			
			iterator++;
		}
	}
	
	return false;
}

bool GoInventory :: IsMeleeWeaponEquipped () const
{
	Go * item;
	
	item = GetEquipped (es_shield_hand);
	if (item != NULL)
	{
		if (item->IsMeleeWeapon())
		{
			return true;
		}
	}
	
	item = GetEquipped (es_shield_hand);
	if (item != NULL)
	{
		if (item->IsMeleeWeapon())
		{
			return true;
		}
	}
	
	return false;
}

bool GoInventory :: IsRangedWeaponEquipped () const
{
	Go * item;
	
	item = GetEquipped (es_shield_hand);
	if (item != NULL)
	{
		if (item->IsRangedWeapon())
		{
			return true;
		}
	}
	
	item = GetEquipped (es_shield_hand);
	if (item != NULL)
	{
		if (item->IsRangedWeapon())
		{
			return true;
		}
	}
	
	return false;
}

bool GoInventory :: IsSlotEquipped (eEquipSlot slot) const
{
	map<eEquipSlot, Go *>::const_iterator iterator = m_equipment.find (slot);
	return iterator != m_equipment.end();
}

const GopSet & GoInventory :: ListItems () const
{
	return m_inventory;
}

bool GoInventory :: Remove (Go * item)
{
	cout << "remove 1" << endl;
	if (item != NULL)
	{
		cout << "remove 2" << endl;
		if (Contains (item) != false)
		{
			cout << "remove 3 ID: " << item->Goid() << endl;
			/*
			 * check if we can actually remove this item 
			 */
			
			m_inventory.erase (item);
			
			item->ClearParent();
			m_go->RemoveChild (item);
			
			return true;
		}
	}
	
	return false;
}

bool GoInventory :: Unequip (eEquipSlot slot)
{
	map<eEquipSlot, Go *>::iterator iterator = m_equipment.find (slot);
	if (iterator != m_equipment.end())
	{
		/*
		 * check if we can actually unequip this item
		 */
		
		m_equipment.erase (iterator);
		
		return true;
	}
	
	return false;
}
