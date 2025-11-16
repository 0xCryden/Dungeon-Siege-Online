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
#include "../Common.h"

#include "Go.hpp"
#include "GoDefend.hpp"

GoDefend :: GoDefend (Go * go) : GoComponent (go)
{
}

GoDefend :: GoDefend (Go * go, xmlNode * node) : GoComponent (go)
{
	if (node != NULL)
	{
		xmlNode * current = NULL;
		for (current = node->children; current != NULL; current = current->next)
		{
			if (current->type != XML_ELEMENT_NODE) continue;
			
			if (xmlStrEqual (current->name, (const xmlChar *) "defense") != 0)
			{
				m_defense = xml::ReadAttribute<float> (current, "value", 0.0);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "defend_class") != 0)
			{
				m_defend_class = StringToDc(xml::XReadString (current, "value", "dc_skin"));
			}
		}
	}
}

float GoDefend :: Defense () const
{
	return m_defense;
}

float GoDefend::GetTotalDefense()
{
	bool log = false;

	float total_defense = 0;

	//Go * owner = GetGo();
	//float base_defense = Defense();
	//total_defense += base_defense;

	//if (log) std::cout << "[GetTotalDefense] Base defense: " << base_defense << std::endl;

	float tgt_dexterity = GetGo()->Actor()->GetSkillLevel("dexterity");
	float tgt_dexterity_bonus = ((tgt_dexterity * 3.5f) + 9.0f);
	total_defense += tgt_dexterity_bonus;

	if (log) {
		std::cout << "[GetTotalDefense] Dexterity: " << tgt_dexterity << std::endl;
		std::cout << "[GetTotalDefense] Dexterity bonus: " << tgt_dexterity_bonus << std::endl;
	}

	// add equipment defense
	for (int i = 0; i < 12; i++)
	{
		Go* equipment = GetGo()->Inventory()->GetEquipped((eEquipSlot)i);

		if (equipment != NULL && equipment->HasDefend())
		{
			float equip_def = equipment->Defend()->Defense();
			total_defense += equip_def;

			if (log) {
				std::cout << "[GetTotalDefense] Slot " << i << " equipment defense: " << equip_def
				          << " (template: " << equipment->TemplateName() << ")" << std::endl;
			}
		}
		else if (log && equipment != NULL)
		{
			std::cout << "[GetTotalDefense] Slot " << i << " has equipment without Defend(). Template: "
			          << equipment->TemplateName() << std::endl;
		}
	}

	if (log) std::cout << "[GetTotalDefense] Total defense: " << total_defense << std::endl;

	return total_defense;
}


float GoDefend :: GetTotalFireResistance ()
{
	return m_defense;
}
