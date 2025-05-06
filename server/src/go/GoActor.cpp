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

GoActor :: GoActor (Go * go) : GoComponent (go)
{
}

GoActor :: GoActor (Go * go, xmlNode * node) : GoComponent (go)
{
	if (node != NULL)
	{
		xmlNode * current = NULL;
		for (current = node->children; current != NULL; current = current->next)
		{
			if (current->type != XML_ELEMENT_NODE) continue;
			
			if (xmlStrEqual (current->name, (const xmlChar *) "alignment") != 0)
			{
				string alignment = xml::XReadString (current, "value", "aa_neutral");
				if (FromString (alignment, m_alignment) != true) m_alignment = aa_neutral;
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "skills") != 0)
			{
				xmlNode * child = NULL;
				for (child = current->children; child != NULL; child = child->next)
				{
					if (child->type != XML_ELEMENT_NODE) continue;
					
					if (xmlStrEqual (child->name, (const xmlChar *) "skill") != 0)
					{
						string name = (const char *) child->name;
						
						map<string, Skill *>::iterator iterator = m_skills.find (name);
						if (iterator == m_skills.end())
						{
							Skill * skill = new Skill;
							skill->name = xml::XReadString (child, "name", name);
							skill->level = xml::ReadAttribute<float> (child, "level", 0.0);
							skill->experience = xml::ReadAttribute<u_int64_t> (child, "experience", 0);
							
							m_skills[skill->name] = skill;
						}
					}
				}
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
        "strength", "intelligence", "dexterity",
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

float GoActor :: GetSkillLevel (const string & skill) const
{
	map<string, Skill *>::const_iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		return iterator->second->level;
	}
	
	return 0.0;
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
