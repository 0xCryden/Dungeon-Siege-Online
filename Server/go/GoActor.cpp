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
#include "Go.hpp"
#include "GoActor.hpp"
#include "../Engine.hpp"
#include "../helper/Helper.h"

static const vector<float> experience_table = {
    0.0f, 70.0f, 650.0f, 1450.0f, 2350.0f, 3350.0f, 4650.0f, 6650.0f, 9450.0f, 12950.0f,
    17100.0f, 22000.0f, 27800.0f, 34300.0f, 41300.0f, 49191.0f, 58327.0f, 68882.0f,
    81054.0f, 95071.0f, 111190.0f, 129706.0f, 150954.0f, 175317.0f, 203567.0f,
    236338.0f, 274352.0f, 318449.0f, 369600.0f, 428936.0f, 497766.0f, 577609.0f,
    670226.0f, 777662.0f, 902288.0f, 1046855.0f, 1214551.0f, 1409080.0f, 1634732.0f,
    1896489.0f, 2200128.0f, 2552348.0f, 2960924.0f, 3434872.0f, 3984651.0f,
    4622395.0f, 5362179.0f, 6220327.0f, 7215779.0f, 8370504.0f, 9709985.0f,
    11263782.0f, 13066188.0f, 15156978.0f, 17582294.0f, 20395661.0f, 23659167.0f,
    27444833.0f, 31836207.0f, 36930200.0f, 42839232.0f, 49780637.0f, 58283859.0f,
    68700306.0f, 81460453.0f, 97091634.0f, 116239830.0f, 139696370.0f, 168430631.0f,
    203630102.0f, 246749453.0f, 299570658.0f, 364276635.0f, 443541456.0f,
    540640862.0f, 659587634.0f, 805297430.0f, 983791930.0f, 1202447693.0f,
    1470301002.0f, 1798421306.0f, 2200368678.0f, 2692754209.0f, 3295926485.0f,
    4034812522.0f, 4939947918.0f, 6048738778.0f, 7407007581.0f, 9070886865.0f,
    11109138988.0f, 13605997839.0f, 16664649931.0f, 20411498744.0f, 25001388540.0f,
    30624003540.0f, 37511706915.0f, 45949143550.0f, 56285003427.0f, 68946431776.0f,
    84456681504.0f, 103456737421.0f, 126731805919.0f, 155243764829.0f,
    190170914494.0f, 232956672834.0f, 285369226800.0f, 349574605408.0f,
    428226194203.0f, 524574390477.0f, 642600930913.0f, 781081513558566.0f,
    956824851411821.0f, 1172110440282060.0f, 1435835286648100.0f,
    1758898223446500.0f, 2154650321024540.0f, 2639446640557650.0f
};

GoActor::GoActor(Go* go) : GoComponent(go)
{
	m_alignment = aa_neutral;
	m_can_level_up = false;
}

GoActor::GoActor(Go* go, GoActor* actor) : GoComponent(go)
{
	m_alignment = actor->Alignment();
	m_can_level_up = actor->CanLevelUp();
	m_skills = actor->Skills();
}

GoActor::GoActor(Go* go, xmlNode* node) : GoComponent(go)
{
	m_alignment = aa_neutral;
	m_can_level_up = false;

	if (node != NULL)
	{
		xmlNode* current = NULL;
		for (current = node->children; current != NULL; current = current->next)
		{
			if (current->type != XML_ELEMENT_NODE) continue;

			if (xmlStrEqual(current->name, (const xmlChar*)"alignment") != 0)
			{
				string alignment = xml::XReadString(current, "value", "aa_neutral");
				if (FromString(alignment, m_alignment) != true) m_alignment = aa_neutral;
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"can_level_up") != 0)
			{
				m_can_level_up = xml::ReadAttribute<bool>(current, "value", true);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"skills") != 0)
			{
				xmlNode* child = NULL;
				for (child = current->children; child != NULL; child = child->next)
				{
					if (child->type != XML_ELEMENT_NODE) continue;

					if (xmlStrEqual(child->name, (const xmlChar*)"skill") != 0)
					{
						string name = (const char*)child->name;

						map<string, Skill*>::iterator iterator = m_skills.find(name);
						if (iterator == m_skills.end())
						{
							Skill* skill = new Skill;
							skill->name = xml::XReadString(child, "name", name);
							skill->level = xml::ReadAttribute<float>(child, "level", 0.0);
							skill->experience = xml::ReadAttribute<float>(child, "experience", 0.0);

							m_skills[skill->name] = skill;
						}
					}
				}
			}
		}
	}
}

GoActor::GoActor(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
	m_alignment = aa_neutral;
	m_can_level_up = false;

	if (tmplComp == nullptr)
		return;

	const string* f;

	if (f = tmplComp->GetField("alignment"))
	{
		if (FromString(*f, m_alignment) != true) m_alignment = aa_neutral;
	}
	if (f = tmplComp->GetField("can_level_up"))
	{
		if (FromString(*f, m_can_level_up) != true) m_can_level_up = false;
	}
	if (const TemplateComponent* skillComponent = tmplComp->GetSubcomponent("skills"))
	{
		for (std::unordered_map<string, string>::const_iterator it = skillComponent->fields.begin();
			it != skillComponent->fields.end(); ++it)
		{
			const string& fieldName = it->first;
			const string& fieldValue = it->second;

			map<string, Skill*>::iterator iterator = m_skills.find(fieldName);
			if (iterator == m_skills.end())
			{
				Skill* skill = new Skill;
				skill->name = fieldName;
				try { skill->level = std::stof(skillComponent->SkillLevelString(fieldName)); }
				catch (...) { skill->level = 0.0f; }
				try { skill->experience = std::stof(skillComponent->SkillExpString(fieldName)); }
				catch (...) { skill->experience = 0.0f; }

				m_skills[skill->name] = skill;
			}
		}
	}
}

GoActor :: ~GoActor ()
{
	for (map<string, Skill *>::iterator iterator = m_skills.begin(); iterator != m_skills.end(); iterator++)
	{
		delete iterator->second;
	}
}

void GoActor::InheritFrom(GoActor* parent)
{
	if (m_alignment == aa_neutral)
		m_alignment = parent->Alignment();

	if (m_can_level_up == false)
		m_can_level_up = parent->CanLevelUp();

	if (m_skills.empty())
		m_skills = parent->Skills();
}

map<string, Skill*> GoActor::Skills()
{
	return m_skills;
}

void GoActor::SaveSkills(xmlNode* actorNode) const
{
    // Get or create <skills> node
    xmlNode* skillsNode = nullptr;
    for (xmlNode* child = actorNode->children; child; child = child->next)
    {
    	if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST "skills"))
    	{
    		skillsNode = child;
            break;
        }
    }

    if (!skillsNode) {
    	std::cout << "[GoActor] No skills to save" << std::endl;
    	return;
    }

    const std::string hardcodedSkills[] = {
        "uber", "strength", "intelligence", "dexterity",
        "melee", "ranged", "nature magic", "combat magic"
    };

    // Assuming skillsNode is already your <skills> xmlNode
    for (const std::string& name : hardcodedSkills)
    {
        auto it = m_skills.find(name);
        if (it == m_skills.end()) continue;

        Skill* skill = it->second;
        if (!skill) continue;

        xmlNode* skillNode = nullptr;

        // Search for existing <skill> node by name
        for (xmlNode* child = skillsNode->children; child; child = child->next)
        {
            if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST "skill"))
            {
                std::string existingName = xml::XReadString(child, "name", "");
                if (existingName == name)
                {
                    skillNode = child;
                    break;
                }
            }
        }

        if (!skillNode)
        	std::cout << "Skill '" << name << "' not found" << std::endl;
        // Update attributes
        //xml::SetAttribute(skillNode, "name", skill->name);
        xml::SetAttribute(skillNode, "level", skill->level);
        xml::SetAttribute(skillNode, "experience", skill->experience);
    }
}

void GoActor::Save(xmlNode* actorNode) const
{
	xml::SetOrUpdateChildValue(actorNode, "alignment", ToString(Alignment()));
    SaveSkills(actorNode);
}

eActorAlignment GoActor :: Alignment () const
{
	return m_alignment;
}

bool GoActor :: CanLevelUp ()
{
	return m_can_level_up;
}


float GoActor :: GetLevelFromXP(float xp) {
    const auto& table = experience_table;
    size_t level = 0;

    // Find the highest level such that required XP is <= current XP
    while (level + 1 < table.size() && table[level + 1] <= xp) {
        ++level;
    }

    // If max level reached or exact match
    if (level + 1 >= table.size() || table[level] == xp) {
        return static_cast<float>(level);
    }

    // Interpolate fractional part
    float xp_start = table[level];
	float xp_end   = table[level + 1];
    float fraction = static_cast<float>(xp - xp_start) / (xp_end - xp_start);

    return static_cast<float>(level) + fraction;
}

float GoActor :: GetXPFromLevel(float level) {
    const auto& table = experience_table;
    int baseLevel = static_cast<int>(floor(level));

    if (baseLevel >= static_cast<int>(table.size()) - 1) {
        return table.back(); // Clamp to max XP in table
    }

    float fraction = level - baseLevel;
	float xp_start = table[baseLevel];
	float xp_end = table[baseLevel + 1];

    return (xp_start + fraction * (xp_end - xp_start));
}

float  GoActor :: GetMaxExpGainForLevel(float level)
{
    static const std::vector<float> maxExpPerLevel = {
        7.00f,14.50f,20.00f,22.50f,25.00f,32.50f,50.00f,70.00f,87.50f,103.75f,122.50f,145.00f,162.50f,175.00f,197.28f,228.40f,
        263.88f,304.30f,350.43f,402.98f,462.90f,531.20f,609.08f,706.25f,819.28f,950.35f,1102.42f,1278.78f,1483.40f,1720.75f,
        1996.08f,2315.43f,2685.90f,3115.65f,3614.18f,4192.40f,4863.23f,5641.30f,6543.93f,7590.98f,8805.50f,10214.40f,11848.70f,
        13744.48f,15943.60f,18494.60f,21453.70f,24886.30f,28868.12f,33487.03f,38844.93f,45060.15f,52269.75f,60632.90f,70334.18f,
        81587.65f,94641.65f,109784.35f,127349.83f,147725.80f,173535.12f,212580.55f,260411.18f,319003.68f,390779.53f,478704.90f,
        586413.50f,718356.53f,879986.78f,1077983.78f,1320530.12f,1617649.43f,1981620.53f,2427485.15f,2973669.30f,3642744.90f,
        4462362.50f,5466394.08f,6696332.73f,8203007.60f,10048684.30f,12309638.28f,15079306.90f,18472150.93f,22628384.90f,
        27719771.50f,33956720.08f,41596982.10f,50956303.08f,62421471.28f,76466302.30f,93671220.33f,114747244.90f,140565375.00f,
        172192584.38f,210935915.88f,258396496.93f,316535708.73f,387756243.20f,475001397.93f,581876712.45f,712798972.75f,
        873178741.62f,1069643958.50f,1310313849.15f,1605134465.20f,1966289719.88f,2408704906.85f,2950663510.90f,3614562800.83f,
        4427839431.05f,5424103303.03f,6644526546.18f,8139545019.10f,9970942648.38f,12214404744.28f,14962645811.73f,
        18329241119.38f,22453320371.23f,27505317454.78f,33694013882.08f,41275167005.53f,50562079581.80f,61938547487.70f,
        75874720672.43f,92946532823.70f,113859502709.08f,139477890818.58f,170860416252.78f,209304009909.68f,256397412139.30f,
        314086829870.67f,384756366591.58f,471326549074.68f,577375022616.47f,707284402705.20f,866423393313.85f,1061368656809.48f,
        1300176604591.60f,1592716340624.70f,1951077517265.28f,2390069958649.98f,2927835699346.20f,3586598731699.10f,
        4393583446331.38f,5382139721755.98f,6593121159151.00f,8076573419960.00f,9893802439451.00f
    };

    int index = static_cast<int>(floor(level));

    // Clamp index to array bounds
    if (index < 0)
        index = 0;
    else if (index >= static_cast<int>(maxExpPerLevel.size()))
        index = static_cast<int>(maxExpPerLevel.size()) - 1;

    return maxExpPerLevel[index];
}
void GoActor :: ChangeSkillLevel (const string & skill, float delta)
{
	map<string, Skill *>::iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		iterator->second->level =+ delta;
		if (iterator->second->level < 0.0)
		{
			iterator->second->level = 0.0;
		}
	}
}

bool GoActor :: HasSkill (const string & skill) const
{
	map<string, Skill *>::const_iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		return true;
	}
	return false;
}

float GoActor :: GetSkillLevel (const string & skill) const
{
	map<string, Skill *>::const_iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		return iterator->second->level;
	}
	
	return 0.0;
}

float GoActor :: GetSkillExp (const string & skill) const
{
	map<string, Skill *>::const_iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		return iterator->second->experience;
	}
	return 0;
}

float GoActor :: HighestSkillLevel () const
{
	float highest = 0.0;
	for (map<string, Skill *>::const_iterator iterator = m_skills.begin(); iterator != m_skills.end(); iterator++)
	{
		if (iterator->second->level > highest)
		{
			return iterator->second->level;
		}
	}
	
	return highest;
}

void GoActor :: SetAlignment (eActorAlignment alignment)
{
	m_alignment = alignment;
}

void GoActor :: SetSkillLevel (const string & skill, float value)
{
	map<string, Skill *>::iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		iterator->second->level = value;
	}
}

void GoActor :: SetSkillExp (const string & skill, float value)
{
	map<string, Skill *>::iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		iterator->second->experience = value;
	}
}

void GoActor :: AddSkillExp (const string & skill, float value)
{
	map<string, Skill *>::iterator iterator = m_skills.find (skill);
	if (iterator == m_skills.end())
		return;

	if ((skill == "melee") && (GetGo()->Inventory()->IsMeleeWeaponEquipped() == false))
	{
		AddSkillExp("uber", value);
		return;
	}

	Skill* skillPtr = iterator->second;
	skillPtr->experience += value;

	// Prevent overflow beyond table
	if (skillPtr->experience > experience_table.back()) {
		skillPtr->experience = experience_table.back();
	}

	// Get new level based on XP
	float newLevel = GetLevelFromXP(skillPtr->experience);
	float oldLevel = skillPtr->level;
	cout << "Adding XP to:" << skill << " amount: " << value << " level before: " << oldLevel << " level after: " << newLevel << endl;
	// Store new level
	skillPtr->level = newLevel;

	// Trigger level-up behavior (only if whole level increased)
	if (std::floor(newLevel) > std::floor(oldLevel)) {
		if (skill != "uber") {
			g_engine.UpdateGoLvlup(GetGo(), skill);
			GetGo()->CalculateStatus();
		}
	}

	// Attribute influence logic
	int str_influence = 0;
	int dex_influence = 0;
	int int_influence = 0;

	if (skill == "melee") {
		str_influence = 64;
		dex_influence = 27;
		int_influence = 9;
		AddSkillExp("uber", value);
	}
	else if (skill == "ranged") {
		str_influence = 25;
		dex_influence = 62;
		int_influence = 13;
		AddSkillExp("uber", value);
	}
	else if (skill == "nature magic") {
		str_influence = 9;
		dex_influence = 18;
		int_influence = 73;
		AddSkillExp("uber", value);
	}
	else if (skill == "combat magic") {
		str_influence = 13;
		dex_influence = 17;
		int_influence = 70;
		AddSkillExp("uber", value);
	}
	else if (skill == "strength" || skill == "dexterity" || skill == "intelligence") {
		// Pull attribute influences from "uber" level instead
		// No redistribution
		return;
	}

	if (str_influence + dex_influence + int_influence > 0) {
		AddSkillExp("strength",     (value * str_influence) / 100.0f);
		AddSkillExp("dexterity",    (value * dex_influence) / 100.0f);
		AddSkillExp("intelligence", (value * int_influence) / 100.0f);
	}
}
