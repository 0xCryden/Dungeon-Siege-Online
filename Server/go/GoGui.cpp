#include "../Common.h"

#include "Go.hpp"
#include "GoGui.h"
#include "../enum/eEquipSlot.hpp"

GoGui::GoGui(Go* go) : GoComponent(go)
{
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

GoGui::GoGui(Go* go, const std::map<std::string, std::string>& r) : GoComponent(go)
{
    m_equip_slot = StringToNum(r.at("equip_slot"));
    m_inventory_width = std::stoi(r.at("inventory_width"));
    m_inventory_height = std::stoi(r.at("inventory_height"));

    // equip_requirements is serialized; handle parsing if needed
    // For now, store as string pointer map placeholder
    // You can implement actual parsing elsewhere
    // Example: parse "skill1:1.0;skill2:2.5" into m_equip_requirements
}

void GoGui::Save(MySQL& db)
{
    std::string q =
        "INSERT INTO t_go_gui (go_id, equip_slot, inventory_width, inventory_height, equip_requirements) VALUES (" +
        std::to_string(GetGo()->Goid()) + ", '" +
        ToString(m_equip_slot) + "', " +
        std::to_string(m_inventory_width) + ", " +
        std::to_string(m_inventory_height) + ", '') "
        "ON DUPLICATE KEY UPDATE "
        "equip_slot=VALUES(equip_slot), "
        "inventory_width=VALUES(inventory_width), "
        "inventory_height=VALUES(inventory_height), "
        "equip_requirements=VALUES(equip_requirements)";

    db.AsyncQuery(q, [](const auto&) {});
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