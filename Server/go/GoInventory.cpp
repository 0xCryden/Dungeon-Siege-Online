#include "../Common.h"
#include "GoDb.hpp"
#include "GoInventory.hpp"
#include "../map/WorldMap.h"
#include "../msg/WorldMessage.h"

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

GoInventory::GoInventory(Go* newGo, const GoInventory& other) : GoComponent(newGo) // attach to new Go
{
	m_custom_head = other.m_custom_head;
	m_selected_active_location = other.m_selected_active_location;

	// Copy inventory items
	for (Go* item : other.m_inventory)
	{
		if (item)
		{
			// Shallow copy pointer
			//m_inventory.insert(item);

			// OR deep copy if needed:
			m_inventory.insert(godb.CloneGo(item->TemplateName()));
		}
	}

	// Copy equipped items
	for (const auto& [slot, item] : other.m_equipment)
	{
		m_equipment[slot] = item;
	}
}

GoInventory :: GoInventory (Go * go, xmlNode * node) : GoComponent (go)
{
	if (node == NULL)
		return;

	m_custom_head = "";

	for (xmlNode * current = node->children; current != NULL; current = current->next)
	{
		if (current->type != XML_ELEMENT_NODE) continue;

		if (xmlStrEqual(current->name, (const xmlChar *) "custom_head") != 0)
		{
			m_custom_head = xml::XReadString (current, "value", "");
		}
		else if (xmlStrEqual(current->name, (const xmlChar *) "item") != 0)
		{
			uint32_t id = xml::ReadAttribute<uint32_t> (current, "id", 0);
			std::string equipSlotStr = xml::ReadAttribute<std::string>(current, "equip_slot", "");
			std::string invenLocStr = xml::ReadAttribute<std::string>(current, "inven_loc", "");

			Go* item = nullptr;
			item = godb.FindGoById(id);
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
}

GoInventory::GoInventory(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
	if (tmplComp == nullptr)
		return;

	const string* f;

	//m_custom_head = "";
	//m_selected_active_location = il_active_melee_weapon;

	if (f = tmplComp->GetField("custom_head")) { try { m_custom_head = *f; } catch (...) { m_custom_head = ""; } }
	if (f = tmplComp->GetField("selected_active_location")) { if (FromString(*f, m_selected_active_location) != true) m_selected_active_location = il_active_melee_weapon; }

	if (const TemplateComponent* equipComponent = tmplComp->GetSubcomponent("equipment"))
	{
		for (std::unordered_map<string, string>::const_iterator it = equipComponent->fields.begin();
			it != equipComponent->fields.end(); ++it)
		{
			const string& equipSlot = it->first;
			const string& originString = it->second;
			string itemTemplateName = it->second;
			string pContent = "";

			if (originString[0] == '#')
			{
				size_t colonPos = originString.find(':');

				if (colonPos != string::npos && colonPos > 1 && colonPos < originString.size() - 1)
				{
					pContent = originString; // pContent = templateName.substr(colonPos + 1);
					itemTemplateName = originString.substr(1, colonPos - 1); // skip '#'
				}
				else
				{
					cout << "[error] Invalid format. Expected #<template>:<value>\n";
					continue;
				}
			}
			TemplateData* itemTmpl = nullptr;
			itemTmpl = manager.GetTemplate(itemTemplateName);
			if (!itemTmpl) // error template not found
			{
				cout << "ERROR item template not found for inventory of go. name: " << itemTemplateName << endl;
				continue;
			}

			Go* item = nullptr;
			//item = godb.CloneGo(itemTemplateName);
			item = new Go(*itemTmpl, pContent);
			if (!item) // error template not found
			{
				cout << "ERROR item template not found for inventory of go" << endl;
				continue;
			}

			if (Add(item))
			{
				// Set inventory location
				item->SetLoc(item->IntendedLoc());

				// If equip_slot is present, equip it
				eEquipSlot slot = StringToNum(equipSlot); // You need to implement this
				if (slot != es_none)
				{
					Equip(slot, item); // Will only succeed if not already equipped
				}
			}
		}
	}
	// TODO add [other]
	// TODO add [pcontent]
}

void GoInventory::InheritFrom(const GoInventory& other)
{
	// Merge inventory items
	m_inventory.insert(other.m_inventory.begin(), other.m_inventory.end());

	// Merge equipped items (only if slot empty)
	for (const auto& [slot, item] : other.m_equipment)
	{
		if (m_equipment.find(slot) == m_equipment.end() && item)
		{
			m_equipment[slot] = item;
		}
	}

	// Merge custom head
	if (m_custom_head.empty())
		m_custom_head = other.m_custom_head;

	// Merge selected active location
	if (m_selected_active_location == il_active_melee_weapon) // default value
		m_selected_active_location = other.m_selected_active_location;
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
		else if (current->type == XML_ELEMENT_NODE && xmlStrEqual(current->name, BAD_CAST "custom_head"))
		{
			xml::SetAttribute(current, "value", GetCustomHead());
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

std::string GoInventory :: GetCustomHead () const
{
	return m_custom_head;
}

bool GoInventory :: HasCustomHead () const
{
	return m_custom_head != "";
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

void GoInventory::Transfer(Go* item, Go* container, eInventoryLocation loc)
{
    std::cout << "GoInventory::Transfer called" << std::endl;

    if (item == nullptr)
    {
        std::cerr << "[ERROR] Transfer failed: item is NULL" << std::endl;
        return;
    }

    if (container == nullptr)
    {
        std::cerr << "[ERROR] Transfer failed: container is NULL" << std::endl;
        return;
    }

    if (!container->HasInventory())
    {
        std::cerr << "[ERROR] Transfer failed: container " << container->Goid() << " has no inventory" << std::endl;
        return;
    }

    std::cout << "[INFO] Transferring item " << item->Goid()
              << " to container " << container->Goid()
              << " at location " << static_cast<int>(loc) << std::endl;

    // Remove from previous container if needed
    Go* parent = item->Parent();
    if (parent != nullptr)
    {
        std::cout << "[INFO] Item " << item->Goid() << " has parent " << parent->Goid() << std::endl;

        if (parent->HasInventory())
        {
            std::cout << "[INFO] Removing item " << item->Goid() << " from parent inventory (Goid " << parent->Goid() << ")" << std::endl;
            parent->Inventory()->Remove(item);
        }
        else
        {
            std::cout << "[WARN] Parent " << parent->Goid() << " has no inventory — cannot remove" << std::endl;
        }
    }
    else
    {
        std::cout << "[INFO] Item " << item->Goid() << " has no parent" << std::endl;
    }

    // Add to new container
    GoInventory* containerInv = container->Inventory();
    if (containerInv)
    {
        std::cout << "[INFO] Adding item " << item->Goid() << " to new container inventory (Goid " << container->Goid() << ")" << std::endl;
        containerInv->Add(item);
    }
    else
    {
        std::cerr << "[ERROR] container->Inventory() returned NULL for Goid " << container->Goid() << std::endl;
        return;
    }

    // Set location
    item->SetLoc(loc);
    std::cout << "[INFO] SetLoc: Item " << item->Goid()
              << " now at " << static_cast<int>(loc)
              << " inside container " << container->Goid() << std::endl;
}


bool GoInventory :: Equip (eEquipSlot slot, Go * item)
{
	cout << "####### GoInventory.Equip item: " << item->TemplateName() << " for Go: " << GetGo()->Common()->ScreenName() << " in slot " << (int)slot << endl;

	if (IsSlotEquipped (slot) != false) return false;
	if (IsEquipped (item) != false) return false;
	if (Contains (item) != true) return false;
	
	if (item == NULL) return false;

	/*
	 * check if we can actually equip this item
	 */

	m_equipment[slot] = item;

	//reset location if location was occupied
	if (item->IntendedLoc() != il_main)
	{
		if (ItemFromLocation(item->IntendedLoc()))
		{
			ItemFromLocation(item->IntendedLoc())->SetLoc(il_main);
		}
	}

	//set location
	item->SetLoc(item->IntendedLoc());

	if (slot == es_weapon_hand || slot == es_shield_hand)
	{
		if (item->IsRangedWeapon())
			SetSelectedSlot(il_active_ranged_weapon);
		else
			SetSelectedSlot(il_active_melee_weapon);
	}
		
	return true;
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

void GoInventory::SetSelectedSlot(eInventoryLocation num)
{
	// when selecting a new slot:
	// change m_equipment
	// keep items inventory location
    std::cout << "[SetSelectedSlot] Called with num = " << num << std::endl;

    if (num == il_active_primary_spell || num == il_active_secondary_spell) // if switching from weapon to spell unequip
    {
        std::cout << "[SetSelectedSlot] Switching to spell slot: " << num << std::endl;

        Go* spellbook = GetEquipped(es_spellbook);
        if (spellbook)
        {
            std::cout << "[SetSelectedSlot] Found equipped spellbook: " << spellbook->Goid() << std::endl;

            GoInventory* spellInv = spellbook->Inventory();
            if (spellInv)
            {
                std::cout << "[SetSelectedSlot] Spellbook has inventory" << std::endl;

                Go* spellItem = spellInv->ItemFromLocation(num);
                if (spellItem)
                {
                    std::cout << "[SetSelectedSlot] Found spell item at location " << (int)num << ": " << spellItem->Goid() << std::endl;

                    GoMagic* magic = spellItem->Magic();
                    if (magic)
                    {
                        float required = magic->RequiredLevel();
                        float actual = GetGo()->Actor()->GetSkillLevel(magic->SkillClass());

                        std::cout << "[SetSelectedSlot] Spell requires level " << required
                                  << ", actor has level " << actual << std::endl;

                        if (required > actual)
                        {
                            std::cout << "[SetSelectedSlot] Requirement not met. Setting m_selectedSlot to " << num << std::endl;
                            //m_selectedSlot = num;
                            return;
                        }
                    }
                    else
                    {
                        std::cerr << "[SetSelectedSlot] ERROR: Spell item has no Magic component" << std::endl;
                    }
                }
                else
                {
                    std::cerr << "[SetSelectedSlot] No spell item at location " << (int)num << std::endl;
                }
            }
            else
            {
                std::cerr << "[SetSelectedSlot] ERROR: Spellbook has no inventory" << std::endl;
            }
        }
        else
        {
            std::cerr << "[SetSelectedSlot] No equipped spellbook" << std::endl;
        }

        if (m_selected_active_location == il_active_melee_weapon)
        {
            std::cout << "[SetSelectedSlot] Currently selected slot is il_active_melee_weapon" << std::endl;

            Go* weapon = GetEquipped(es_weapon_hand);
            if (!weapon || weapon->Attack()->AttackClass() != ac_staff)
            {
                std::cout << "[SetSelectedSlot] Weapon is not a staff (or none equipped), unequipping weapon and shield hand" << std::endl;
                Unequip(es_weapon_hand);
                Unequip(es_shield_hand);
            }
            else
            {
                std::cout << "[SetSelectedSlot] Equipped weapon is a staff, no unequip" << std::endl;
            }
        }
        else if (m_selected_active_location == il_active_ranged_weapon)
        {
            std::cout << "[SetSelectedSlot] Currently selected slot is 2, unequipping shield hand" << std::endl;
            Unequip(es_shield_hand);
        }
    }
	/*else if (num == il_active_melee_weapon)
	{
		//reequip item in slot
		Go* slotItem = ItemFromLocation(num);
		if (slotItem != NULL)
		{
			GetGo()->Mind()->Equip(slotItem->Gui()->EquipSlot(), slotItem);
		}
	}
	else if (num == il_active_ranged_weapon)
	{
		Go* slotItem = ItemFromLocation(num);
		if (slotItem != NULL)
		{
			GetGo()->Mind()->Equip(slotItem->Gui()->EquipSlot(), slotItem);
		}
	}*/


    //std::cout << "[SetSelectedSlot] Finalizing: setting m_selectedSlot to " << num << std::endl;
	m_selected_active_location = num;
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
		cout << "####### GoInventory.Unquip item: " << iterator->second->Aspect()->Model() << " for Go: " << GetGo()->Common()->ScreenName() << " from slot " << (int)slot << endl;
		m_equipment.erase (iterator);
		
		return true;
	}
	
	return false;
}
