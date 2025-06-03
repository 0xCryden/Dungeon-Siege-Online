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
			// means its either a template or not in DB
			//cout << "Failed loading item. ID: " << id << " | OwnerGoId: " << m_go->Goid() << endl;
			continue;
		}

		// Add to inventory
		if (Add(item))
		{
			// Set inventory location
			eInventoryLocation invenLoc = StringToNumLoc(invenLocStr); // You already have this

			item->SetLoc(invenLoc);

			// If equip_slot is present, equip it
			if (!equipSlotStr.empty())
			{
				eEquipSlot slot = StringToNum(equipSlotStr); // You need to implement this
				if (slot != es_none)
				{
					Equip(slot, item); // Will only succeed if not already equipped
				}
			}
		}
	}
}

void GoInventory :: Save (xmlNode* inventoryNode) const
{
	//cout << "Entering save inventory" << endl;

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
		std::string slotStr = ToString(slot);
		if (!slotStr.empty())
			xml::SetAttribute(itemNode, "equip_slot", slotStr);
		else
			xml::SetAttribute(itemNode, "equip_slot", "es_none");

		eInventoryLocation loc = item->GetLoc();
		std::string slotStr2 = ToString(loc);
		xml::SetAttribute(itemNode, "inven_loc", slotStr2);
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

void GoInventory :: Transfer (Go * item, Go * container, eInventoryLocation loc)
{
	//cout << "GoInventory :: Transfer" << endl;
	if (item == NULL)
		return;
	if (container == NULL)
		return;
	if (!container->HasInventory())
		return;

	// Remove from previous container if needed
	if (item->Parent() != 0)
	{
		if (item->Parent()->HasInventory())
			item->Parent()->Inventory()->Remove(item);
	}
	// Update in owner's inventory
	container->Inventory()->Add(item);

	// Set new state
	item->SetLoc(loc);

	//std::cout << "[SET ITEM LOC] Item " << item->Goid() << " now at " << (eInventoryLocation)loc << " inside " << container->Goid() << std::endl;
	return;
}

bool GoInventory :: Equip (eEquipSlot slot, Go * item)
{
	cout << "Inventory Equip item: " << item->Aspect()->Model() << endl;

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

		//set location
		item->SetLoc(item->IntendedLoc());
		if (slot == es_weapon_hand)
		{
			SetSelectedSlot(1);
		}
		else if (slot == es_shield_hand)
		{
			if (item->IsRangedWeapon())
			{
				SetSelectedSlot(2);
			}
			else
			{
				SetSelectedSlot(1);
			}
		}
		
		return true;
	}
	
	return false;
}

Go * GoInventory :: GetEquipped (eEquipSlot slot) const
{
	map<eEquipSlot, Go *>::const_iterator iterator = m_equipment.find (slot);
	return iterator != m_equipment.end() ? iterator->second : NULL;
}

Go * GoInventory :: ItemFromLocation (eInventoryLocation loc) const
{
	for (GopSet::const_iterator it = m_inventory.begin(); it != m_inventory.end(); ++it)
	{
		Go* item = *it;
		if (!item)
			continue;

		if (item->GetLoc() == loc)
			return item;
	}
	cout << "Item in loc " << (eInventoryLocation)loc << " not found" << endl;
	return NULL;
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

void GoInventory :: SetSelectedSlot ( int num )
{
	//cout << "Step 1 num: " << num << endl;
	if (num == 3 || num == 4 ) // if switching from weapon to spell unequip
	{
		if (GetEquipped(es_spellbook) &&
			GetEquipped(es_spellbook)->Inventory()->ItemFromLocation((eInventoryLocation)(num+1)) &&
			GetEquipped(es_spellbook)->Inventory()->ItemFromLocation((eInventoryLocation)(num+1))->Magic()->RequiredLevel() > GetGo()->Actor()->GetSkillLevel(GetEquipped(es_spellbook)->Inventory()->ItemFromLocation((eInventoryLocation)(num+1))->Magic()->SkillClass()))
		{
			m_selectedSlot = num;
			return;
		}
		//cout << "Step 2 num: " << num << endl;
		if (m_selectedSlot == 1)
		{
			//cout << "Step 3 num: " << num << endl;
			if (!(GetEquipped(es_weapon_hand) &&
				(GetEquipped(es_weapon_hand)->Attack()->AttackClass() == ac_staff))) // if !item in il_active_melee_weapon attack class == ac_staff
			{
				//cout << "Step 4 num: " << num << endl;
				Unequip(es_weapon_hand);
				Unequip(es_shield_hand);
			}
		}
		else if (m_selectedSlot == 2)
		{
			Unequip(es_shield_hand);
		}

	}
	/*else
	{
		Equip ((eEquipSlot)(num-1), ItemFromLocation((eInventoryLocation)(num-1)));
	}*/

	/*if (num == 1 && ItemFromLocation(il_active_melee_weapon) != NULL)
	{
		Equip (es_weapon_hand, ItemFromLocation(il_active_melee_weapon));
		if (ItemFromLocation(il_shield) != NULL)
			Equip (es_shield_hand, ItemFromLocation(il_shield));
	}
	if (num == 2 && ItemFromLocation(il_active_ranged_weapon) != NULL)
	{
		Equip (es_shield_hand, ItemFromLocation(il_active_ranged_weapon));
	}*/

	m_selectedSlot = num;

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
	
	item = GetEquipped (es_weapon_hand);
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
	
	item = GetEquipped (es_weapon_hand);
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
	if (item != NULL)
	{
		if (Contains (item) != false)
		{
			//cout << "remove 3 ID: " << item->Goid() << endl;
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
	//cout << "GoInventory:: Unequip slot " << ToString(slot) << endl;
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
