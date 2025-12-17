#include "../Common.h"

#include "Go.hpp"
#include "GoGui.h"
#include "../enum/eEquipSlot.hpp"

GoGui::GoGui(Go* go) : GoComponent(go)
{
}

GoGui::GoGui(Go* newGo, const GoGui& other) : GoComponent(newGo) // attach to new Go
{
    m_equip_slot = other.m_equip_slot;
    m_inventory_width = other.m_inventory_width;
    m_inventory_height = other.m_inventory_height;
    m_equip_requirements = other.m_equip_requirements; // assuming std::string or similar
}

GoGui::GoGui(Go* go, xmlNode* node) : GoComponent(go)
{
	if (node != NULL)
	{
		xmlNode* current = NULL;
		for (current = node->children; current != NULL; current = current->next)
		{
			if (current->type != XML_ELEMENT_NODE) continue;

			if (xmlStrEqual(current->name, (const xmlChar*)"equip_slot") != 0)
			{
                m_equip_slot = StringToNum(xml::ReadAttribute<string>(current, "value", "es_none"));
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"inventory_width") != 0)
			{
				m_inventory_width = xml::ReadAttribute<int>(current, "value", 0);
			}
            else if (xmlStrEqual(current->name, (const xmlChar*)"inventory_height") != 0)
            {
                m_inventory_height = xml::ReadAttribute<int>(current, "value", 0);
            }
            else if (xmlStrEqual(current->name, (const xmlChar*)"equip_requirements") != 0)
            {
                std::string reqStr = xml::XReadString(current, "value", "");
                SetEquipRequirements(reqStr);
            }
		}
	}
}

GoGui::GoGui(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
	if (tmplComp == nullptr)
		return;

	const string* f;
	if (f = tmplComp->GetField("equip_slot")) { if (FromString(*f, m_equip_slot) != true) m_equip_slot = es_none; }

	if (f = tmplComp->GetField("inventory_width")) { try { m_inventory_width = static_cast<int>(std::stoi(*f)); } catch (...) { m_inventory_width = 0; } }
	if (f = tmplComp->GetField("inventory_height")) { try { m_inventory_height = static_cast<int>(std::stoi(*f)); } catch (...) { m_inventory_height = 0; } }

    // TODO implement usage
    if (f = tmplComp->GetField("equip_requirements")) { SetEquipRequirements(*f); }
}

void GoGui::InheritFrom(const GoGui& other)
{
    // Merge map: add keys from 'other' if they don't exist in this
    for (const auto& [key, value] : other.m_equip_requirements)
    {
        if (m_equip_requirements.find(key) == m_equip_requirements.end())
        {
            m_equip_requirements[key] = value;
        }
    }
    if (m_equip_slot == es_none)                 m_equip_slot = other.m_equip_slot;
    if (m_inventory_width == 0)                  m_inventory_width = other.m_inventory_width;
    if (m_inventory_height == 0)                 m_inventory_height = other.m_inventory_height;
}

void GoGui::SetEquipRequirements(const std::string& input)
{
    // Example input: "strength:16,dexterity:26;"
    std::string str = input;

    // Remove trailing semicolon if it exists
    if (!str.empty() && str.back() == ';')
        str.pop_back();

    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, ',')) // split at commas
    {
        // Each token should be: "strength:16"
        size_t pos = token.find(':');
        if (pos == std::string::npos)
            continue;

        std::string skillName = token.substr(0, pos);
        std::string valueStr = token.substr(pos + 1);

        TrimEqRequirements(skillName);
        TrimEqRequirements(valueStr);

        float value = 0.0f;
        try {
            value = std::stof(valueStr);
        }
        catch (...) {
            value = 0.0f;
        }

        // Allocate new float (matches your design)
        float* v = new float(value);
        m_equip_requirements[skillName] = v;
    }
}