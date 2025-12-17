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

#include "../scripts/test_script.hpp"
#include "../Engine.hpp"
#include "Go.hpp"

// for GoDb template creation
Go :: Go (xmlNode * node) : m_parent (NULL), m_actor (NULL), m_aspect (NULL), m_attack (NULL), m_body (NULL), m_common (NULL), m_defend (NULL), m_gui(NULL), m_inventory (NULL), m_magic (NULL), m_mind (NULL), m_placement (NULL)
{
	if (node != NULL)
	{
		m_goid = xml::ReadAttribute<uint32_t> (node, "id", 0);
		m_scid = 0;
		m_admin = xml::ReadAttribute<uint8_t> (node, "admin", 0);
		m_template_name = xml::ReadAttribute<string> (node, "template_name", "");
		m_specializes = "";

		xmlNode * current = NULL;
		for (current = node->children; current != NULL; current = current->next)
		{
			if (current->type != XML_ELEMENT_NODE) continue;
			
			if (xmlStrEqual (current->name, (const xmlChar *) "actor") != 0)
			{
				//Log::WriteF("[Go %u] Creating actor...", m_goid);
				if (m_actor == NULL) m_actor = new GoActor (this, current);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "aspect") != 0)
			{
				//Log::WriteF("[Go %u] Creating aspect...", m_goid);
				if (m_aspect == NULL) m_aspect = new GoAspect (this, current);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "attack") != 0)
			{
				//Log::WriteF("[Go %u] Creating attack...", m_goid);
				if (m_attack == NULL) m_attack = new GoAttack (this, current);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "body") != 0)
			{
				//Log::WriteF("[Go %u] Creating body...", m_goid);
				if (m_body == NULL) m_body = new GoBody (this, current);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "common") != 0)
			{
				//Log::WriteF("[Go %u] Creating common...", m_goid);
				if (m_common == NULL) m_common = new GoCommon (this, current);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"defend") != 0)
			{
				//Log::WriteF("[Go %u] Creating defend...", m_goid);
				if (m_defend == NULL) m_defend = new GoDefend(this, current);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"gui") != 0)
			{
				//Log::WriteF("[Go %u] Creating gui...", m_goid);
				if (m_gui == NULL) m_gui = new GoGui(this, current);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "inventory") != 0)
			{
				//Log::WriteF("[Go %u] Creating inventory...", m_goid);
				if (m_inventory == NULL) m_inventory = new GoInventory (this, current);
				//cout << "////////////////// Inven Found. ID: " << m_goid << endl;
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "magic") != 0)
			{
				//Log::WriteF("[Go %u] Creating magic...", m_goid);
				if (m_magic == NULL) m_magic = new GoMagic (this, current);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "mind") != 0)
			{
				//Log::WriteF("[Go %u] Creating mind...", m_goid);
				if (m_mind == NULL) m_mind = new GoMind (this, current);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "placement") != 0)
			{
				//Log::WriteF("[Go %u] Creating placement...", m_goid);
				if (m_placement == NULL) m_placement = new GoPlacement (this, current);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "scripts") != 0)
			{
				xmlNode * a = NULL;
				for (a = current->children; a != NULL; a = a->next)
				{
					if (a->type != XML_ELEMENT_NODE) continue;
					
					// cout << "found name = " << a->name << endl;
					
					if (xmlStrEqual (a->name, (const xmlChar *) "test_script") != 0)
					{
						// add class test_script to m_scripts
						map<string, GoScriptComponent *>::iterator iterator = m_scripts.find ("test_script");
						if (iterator == m_scripts.end())
						{
							m_scripts["test_script"] = new test_script (this);
						}
					}
				}
			}
		}
	}
}

// for GoDb template creation
Go::Go(const TemplateData& tmpl)
	: m_template_name(tmpl.name), m_specializes(tmpl.specializes),
	m_parent(NULL), m_actor(NULL), m_aspect(NULL), m_attack(NULL),
	m_body(NULL), m_common(NULL), m_defend(NULL), m_gui(NULL),
	m_inventory(NULL), m_magic(NULL), m_mind(NULL), m_placement(NULL)
{
	//m_goid = godb.NextId();
	//m_scid = 0;
	//if (tmpl.scid != "")
	//	m_scid = static_cast<uint32_t>(std::stoul(tmpl.scid, nullptr, 16));

	//m_admin = 0;
	//m_template_name = tmpl.name;

	const TemplateComponent* comp;
	if ((comp = tmpl.GetComponent("actor"))) { m_actor = new GoActor(this, comp); }
	if ((comp = tmpl.GetComponent("aspect"))) { m_aspect = new GoAspect(this, comp); }
	if ((comp = tmpl.GetComponent("attack"))) { m_attack = new GoAttack(this, comp); }
	if ((comp = tmpl.GetComponent("body"))) { m_body = new GoBody(this, comp); }
	if ((comp = tmpl.GetComponent("common"))) { m_common = new GoCommon(this, comp); }
	if ((comp = tmpl.GetComponent("defend"))) { m_defend = new GoDefend(this, comp); }
	// X 2 more subcomponents remaining implementation for mob drops
	if ((comp = tmpl.GetComponent("inventory"))) { m_inventory = new GoInventory(this, comp); }
	if ((comp = tmpl.GetComponent("magic"))) { m_magic = new GoMagic(this, comp); }
	if ((comp = tmpl.GetComponent("mind"))) { m_mind = new GoMind(this, comp); }
	// TODO Pcontent
	if ((comp = tmpl.GetComponent("gui"))) { m_gui = new GoGui(this, comp); }
	// TODO Physics

	if ((comp = tmpl.GetComponent("placement"))) { m_placement = new GoPlacement(this, comp); }
}

Go :: Go (uint32_t id, const Go * go) : m_specializes(""), m_parent(NULL), m_actor(NULL), m_aspect(NULL), m_attack(NULL), m_body(NULL), m_common(NULL), m_defend(NULL), m_gui(NULL), m_inventory(NULL), m_magic(NULL), m_mind(NULL), m_placement(NULL)
{
	m_goid = id;
	m_admin = 0;
	m_scid = 0;
	m_template_name = go->m_template_name;
	m_specializes = go->m_specializes;
	
	if (go->m_actor != NULL) m_actor = new GoActor (this);
	if (go->m_aspect != NULL) m_aspect = new GoAspect (this);
	if (go->m_attack != NULL) m_attack = new GoAttack (this);
	if (go->m_body != NULL) m_body = new GoBody (this);
	if (go->m_common != NULL) m_common = new GoCommon (this);
	if (go->m_defend != NULL) m_defend = new GoDefend(this);
	if (go->m_gui != NULL) m_gui = new GoGui(this);
	if (go->m_inventory != NULL) m_inventory = new GoInventory (this);
	if (go->m_magic != NULL) m_magic = new GoMagic (this);
	if (go->m_mind != NULL) m_mind = new GoMind (this);
	if (go->m_placement != NULL) m_placement = new GoPlacement (this);
}

Go::Go(const TemplateData& tmpl, const PlacementData& placement)
	: m_template_name(tmpl.name), m_specializes(tmpl.specializes),
	m_parent(NULL), m_actor(NULL), m_aspect(NULL), m_attack(NULL),
	m_body(NULL), m_common(NULL), m_defend(NULL), m_gui(NULL),
	m_inventory(NULL), m_magic(NULL), m_mind(NULL), m_placement(NULL)
{
	m_goid = godb.NextId();
	m_scid = 0;
	if (tmpl.scid != "" )
		m_scid = static_cast<uint32_t>(std::stoul(tmpl.scid, nullptr, 16));

	m_admin = 0;
	m_template_name = tmpl.name;

	const TemplateComponent* comp;
	if ((comp = tmpl.GetComponent("actor"))) { m_actor = new GoActor(this, comp); }
	if ((comp = tmpl.GetComponent("aspect"))) { m_aspect = new GoAspect(this, comp); }
	if ((comp = tmpl.GetComponent("attack"))) { m_attack = new GoAttack(this, comp); }
	if ((comp = tmpl.GetComponent("body"))) { m_body = new GoBody(this, comp); }
	if ((comp = tmpl.GetComponent("common"))) { m_common = new GoCommon(this, comp); }
	if ((comp = tmpl.GetComponent("defend"))) { m_defend = new GoDefend(this, comp); }
	// X 2 more subcomponents remaining implementation for mob drops
	if ((comp = tmpl.GetComponent("inventory"))) { m_inventory = new GoInventory(this, comp); }
	if ((comp = tmpl.GetComponent("magic"))) { m_magic = new GoMagic(this, comp); }
	if ((comp = tmpl.GetComponent("mind"))) { m_mind = new GoMind(this, comp); }
	// TODO Pcontent
	if ((comp = tmpl.GetComponent("gui"))) { m_gui = new GoGui(this, comp); }
	// TODO Physics

	m_placement = new GoPlacement(this, placement);

	m_conversations = placement.conversations;
}

Go::Go(const TemplateData& tmpl, const GoPlacement& placement)
	: m_template_name(tmpl.name), m_specializes(""),
	m_parent(NULL), m_actor(NULL), m_aspect(NULL), m_attack(NULL),
	m_body(NULL), m_common(NULL), m_defend(NULL), m_gui(NULL),
	m_inventory(NULL), m_magic(NULL), m_mind(NULL), m_placement(NULL)
{
	m_goid = godb.NextId();
	m_admin = 0;
	m_template_name = tmpl.name;

	const TemplateComponent* comp;
	if ((comp = tmpl.GetComponent("actor"))) { m_actor = new GoActor(this, comp); }
	if ((comp = tmpl.GetComponent("aspect"))) { m_aspect = new GoAspect(this, comp); }
	if ((comp = tmpl.GetComponent("attack"))) { m_attack = new GoAttack(this, comp); }
	if ((comp = tmpl.GetComponent("body"))) { m_body = new GoBody(this, comp); }
	if ((comp = tmpl.GetComponent("common"))) { m_common = new GoCommon(this, comp); }
	if ((comp = tmpl.GetComponent("defend"))) { m_defend = new GoDefend(this, comp); }
	// X 2 more subcomponents remaining implementation for mob drops
	if ((comp = tmpl.GetComponent("inventory"))) { m_inventory = new GoInventory(this, comp); }
	if ((comp = tmpl.GetComponent("magic"))) { m_magic = new GoMagic(this, comp); }
	if ((comp = tmpl.GetComponent("mind"))) { m_mind = new GoMind(this, comp); }
	// TODO Pcontent
	if ((comp = tmpl.GetComponent("gui"))) { m_gui = new GoGui(this, comp); }
	// TODO Physics

	m_placement = new GoPlacement(this, placement);
}

Go::Go(const Go* tmpl, const GoPlacement& placement)
	: m_template_name(tmpl->TemplateName()), m_specializes(""),
	m_parent(nullptr), m_actor(nullptr), m_aspect(nullptr), m_attack(nullptr),
	m_body(nullptr), m_common(nullptr), m_defend(nullptr), m_gui(nullptr),
	m_inventory(nullptr), m_magic(nullptr), m_mind(nullptr), m_placement(nullptr)
{
	m_goid = godb.NextId();
	m_admin = 0;

	// Copy the placement first
	m_placement = new GoPlacement(this, placement);

	// Use the template to fill components
	if (tmpl->m_specializes != "")
	{
		// Recursively inherit from parent template if needed
		m_specializes = tmpl->m_specializes;
		InheritFrom(tmpl->m_specializes);
	}

	if (tmpl->m_actor)
	{
		m_actor = new GoActor(this, *tmpl->m_actor);
	}
	if (tmpl->m_aspect)
	{
		m_aspect = new GoAspect(this, *tmpl->m_aspect);
	}
	if (tmpl->m_attack)
	{
		m_attack = new GoAttack(this, *tmpl->m_attack);
	}
	if (tmpl->m_body)
	{
		m_body = new GoBody(this, *tmpl->m_body);
	}
	if (tmpl->m_common)
	{
		m_common = new GoCommon(this, *tmpl->m_common);
	}
	if (tmpl->m_defend)
	{
		m_defend = new GoDefend(this, *tmpl->m_defend);
	}
	if (tmpl->m_gui)
	{
		m_gui = new GoGui(this, *tmpl->m_gui);
	}
	if (tmpl->m_inventory)
	{
		m_inventory = new GoInventory(this, *tmpl->m_inventory);
	}
	if (tmpl->m_magic)
	{
		m_magic = new GoMagic(this, *tmpl->m_magic);
	}
	if (tmpl->m_mind)
	{
		m_mind = new GoMind(this, *tmpl->m_mind);
	}

	// The placement is already provided, do not override
	// If tmpl has a placement but you want to merge, you could call:
	// if (tmpl->m_placement)
	//     m_placement->InheritFrom(*tmpl->m_placement);

	// TODO: Implement mob drops, pcontent, physics, scripts if needed
}



Go :: ~Go ()
{
	if (HasComponent("player"))
		SaveToXml("actors");
	else
		SaveToXml("items");

	if (m_actor) delete m_actor;
	if (m_aspect) delete m_aspect;
	if (m_attack) delete m_attack;
	if (m_body) delete m_body;
	if (m_common) delete m_common;
	if (m_defend) delete m_defend;
	if (m_gui) delete m_gui;
	if (m_inventory) delete m_inventory;
	if (m_magic) delete m_magic;
	if (m_mind) delete m_mind;
	if (m_placement) delete m_placement;
	
	map<string, GoScriptComponent *>::iterator iterator = m_scripts.begin();
	while (iterator != m_scripts.end())
	{
		delete iterator->second;
	}
}

void Go::InheritFrom(const string& parentName)
{
	Go* parent = godb.FindTemplateByName(parentName);
	if (parent == NULL)
	{
		Log::Write(Log::Level::ERR, "Couldnt find parent template to inherit from ERROR", true);
		return;
	}

	if (parent->m_specializes != "")
	{
		InheritFrom(parent->m_specializes);
	}

	if (parent->m_actor)
	{
		if (!m_actor) m_actor = new GoActor(this, *parent->m_actor);
		else          m_actor->InheritFrom(*parent->m_actor);
	}
	if (parent->m_aspect)
	{
		if (!m_aspect) m_aspect = new GoAspect(this, *parent->m_aspect);
		else           m_aspect->InheritFrom(*parent->m_aspect);
	}
	if (parent->m_attack)
	{
		if (!m_attack) m_attack = new GoAttack(this, *parent->m_attack);
		else           m_attack->InheritFrom(*parent->m_attack);
	}
	if (parent->m_body)
	{
		if (!m_body) m_body = new GoBody(this, *parent->m_body);
		else          m_body->InheritFrom(*parent->m_body);
	}
	if (parent->m_common)
	{
		if (!m_common) m_common = new GoCommon(this, *parent->m_common);
		else           m_common->InheritFrom(*parent->m_common);
	}
	if (parent->m_defend)
	{
		if (!m_defend) m_defend = new GoDefend(this, *parent->m_defend);
		else           m_defend->InheritFrom(*parent->m_defend);
	}
	if (parent->m_gui)
	{
		if (!m_gui) m_gui = new GoGui(this, *parent->m_gui);
		else        m_gui->InheritFrom(*parent->m_gui);
	}
	if (parent->m_inventory)
	{
		if (!m_inventory) m_inventory = new GoInventory(this, *parent->m_inventory);
		else              m_inventory->InheritFrom(*parent->m_inventory);
	}
	if (parent->m_magic)
	{
		if (!m_magic) m_magic = new GoMagic(this, *parent->m_magic);
		else          m_magic->InheritFrom(*parent->m_magic);
	}
	if (parent->m_mind)
	{
		if (!m_mind) m_mind = new GoMind(this, *parent->m_mind);
		else         m_mind->InheritFrom(*parent->m_mind);
	}
	if (parent->m_placement)
	{
		if (!m_placement) m_placement = new GoPlacement(this, *parent->m_placement);
		else              m_placement->InheritFrom(*parent->m_placement);
	}
	/*for (const auto& [name, script] : parent->m_scripts)
	{
		if (m_scripts.count(name) == 0)
		{
			m_scripts[name] = script->CloneFor(this);
		}
		else
		{
			m_scripts[name]->InheritFrom(script);
		}
	}*/
}

double Go :: GetDistanceTo(Go * target)
{
	//cout << "IsInRange begin" << endl;

	if (target == NULL)
	{
		return -1;
	}

	if (target->HasPlacement() != true || HasPlacement() != true)
	{
		return -1;
	}

	if (target->Placement()->GetRegion() != Placement()->GetRegion())
	{
		return -1;
	}

	if (Placement()->GetRegion().empty())
	{
		return -1;
	}

	Region * region = g_world.GetRegion (Placement()->GetRegion());
	double dist = region->GetSiegeDistance (Placement()->Position(), target->Placement()->Position());
	//cout << a->Goid() << " is in range of " << b->Goid() << " | Range: " << (double)dist << endl;
	return dist;
}

void Go :: HandleCommand (const string& command)
{
    // Split the command by space
    istringstream iss(command);
    string cmd;
    iss >> cmd;
    cout << cmd << " received" << endl;
    if (cmd == "/add")
    {
        string templateName;
        iss >> templateName;

        if (templateName.empty())
        {
            cout << "Usage: /add <template_name>" << endl;
            return;
        }

		if (TemplateData* tpl = manager.GetTemplate(templateName))
		{
			cout << "[info] template: " << tpl->name << "\n";
			if (!tpl->specializes.empty())
				cout << "  specializes: " << tpl->specializes << "\n";

			for (const auto& [compname, comp] : tpl->components) {
				gas.LogComponent(compname, comp, "  ");
			}
			// access fields
			/*auto* comp = tpl->getcomponent("aspect");
			if (comp) {
				auto experience_value = comp->getfield("experience_value");
				if (experience_value) {
					std::cout << "experience_value: " << *experience_value << "\n";
				}
			}*/
		}

        // Try to spawn or add using the template name
		godb.SpawnGo(templateName, this);
    }
    else if (cmd == "/setlvl")
    {
    	if (!HasActor())
    		return;

    	/*if (Actor()->CanLevelUp() == false)
    		return;*/

        string levelAmount;
        iss >> levelAmount;

        if (levelAmount.empty())
        {
            cout << "Usage: /setlvl <level> (<skill>)" << endl;
            return;
        }
        else
        {
            string skillName;
            iss >> skillName;

            if (skillName.empty() || skillName == "all") // all skills
            {
            	float levels = stof(levelAmount);

            	Actor()->SetSkillLevel("uber", levels);

            	Actor()->SetSkillLevel("strength", levels);
            	Actor()->SetSkillLevel("dexterity", levels);
            	Actor()->SetSkillLevel("intelligence", levels);

            	Actor()->SetSkillLevel("melee", levels);
            	Actor()->SetSkillLevel("ranged", levels);
            	Actor()->SetSkillLevel("nature magic", levels);
            	Actor()->SetSkillLevel("combat magic", levels);

            	float exp = Actor()->GetXPFromLevel(levels);

            	Actor()->SetSkillExp("uber", exp);

            	Actor()->SetSkillExp("strength", exp);
            	Actor()->SetSkillExp("dexterity", exp);
            	Actor()->SetSkillExp("intelligence", exp);

            	Actor()->SetSkillExp("melee", exp);
            	Actor()->SetSkillExp("ranged", exp);
            	Actor()->SetSkillExp("nature magic", exp);
            	Actor()->SetSkillExp("combat magic", exp);

            	g_engine.UpdateGo(this);

            }
            else // specific skill
            {
            	if (!Actor()->HasSkill(skillName))
            		return;

            	float levels = stof(levelAmount);

            	Actor()->SetSkillLevel(skillName, levels);

            	float exp = Actor()->GetXPFromLevel(levels);

            	Actor()->SetSkillExp(skillName, exp);

            	g_engine.UpdateGo(this);
            }
        }
    }
    else
    {
    	cout << "Unknown command: " << cmd << endl;
    }
}

eEquipSlot Go :: IntendedSlot()
{
	// TODO check eEquipSlot value inherited from template instead

	if (HasGui() && Gui()->EquipSlot() != es_none)
	{
		return Gui()->EquipSlot();
	}

	eEquipSlot slot = es_any;

	if (HasAttack() && IsMeleeWeapon())
	{
		cout << "Mapping item slot to es_weapon_hand" << endl;
		slot = es_weapon_hand;
	}
	if (HasAttack() && IsRangedWeapon())
	{
		cout << "Mapping item slot to es_shield_hand" << endl;
		slot = es_shield_hand;
	}

	if (HasDefend() && Defend()->DefendClass() == dc_shield)
	{
		cout << "Mapping item slot to es_shield_hand" << endl;
		slot = es_shield_hand;
	}

	return slot;
}

eInventoryLocation Go :: IntendedLoc()
{
	eInventoryLocation loc = il_main;

	if (HasAttack() && IsMeleeWeapon())
	{
		//cout << "Mapping item location to il_active_melee_weapon" << endl;
		loc = il_active_melee_weapon;
	}
	if (HasAttack() && IsRangedWeapon())
	{
		//cout << "Mapping item location to il_active_ranged_weapon" << endl;
		loc = il_active_ranged_weapon;
	}

	if (HasDefend() && Defend()->DefendClass() == dc_shield)
	{
		//cout << "Mapping item location to il_shield" << endl;
		loc = il_shield;
	}

	return loc;
}

string Go::GetTitle() {
    struct TitleGroup {
        vector<string> skills;
        vector<string> maleTitles;
        vector<string> femaleTitles; // optional
    };

    vector<TitleGroup> titleGroups = {
        {{"melee", "ranged", "nature magic", "combat magic"},
         {"Freelance", "Journeyman", "Adventurer", "Master", "Grand Master", "Grand High SiegeMaster"},
         {"Freelance", "Journeyman", "Adventurer", "Master", "Grand Master", "Grand High Siegemistress"}},

        {{"melee", "ranged", "combat magic"},
         {"Mercenary", "Gladiator", "Centurion", "Myrmidon", "Warlord", "Warlord Noble"},
         {"Mercenary", "Gladiator", "Centurion", "Myrmidon", "Warlady", "Warlady Noble"}},

        {{"melee", "ranged", "nature magic"},
         {"Cavalier", "Marshal", "Paladin", "Templar", "Arch Templar", "Supreme Templar"}},

        {{"melee", "nature magic", "combat magic"},
         {"Initiate", "Mystic", "Sage", "Deacon", "Grand Deacon", "Grand High Deacon"},
         {"Initiate", "Mystic", "Sage", "Deaconess", "Grand Deaconess", "Grand High Deaconess"}},

        {{"ranged", "nature magic", "combat magic"},
         {"Adept", "Conjurer", "Thaumaturgist", "Evoker", "Senior Evoker", "Lord Evoker"},
         {"Adept", "Conjurer", "Thaumaturgist", "Evoker", "Senior Evoker", "Lady Evoker"}},

        {{"melee", "ranged"},
         {"Man-At-Arms", "Skirmisher", "Raider", "Campaigner", "Crusader", "Grand Crusader"},
         {"Woman-At-Arms", "Skirmisher", "Raider", "Campaigner", "Crusader", "Grand Crusader"}},

        {{"melee", "nature magic"},
         {"Friar", "Curate", "Druid", "Preserver", "Grand Preserver", "Supreme Preserver"}},

        {{"melee", "combat magic"},
         {"Combatant", "Duelist", "Dragoon", "Warlock", "Grand Warlock", "Grand High Warlock"},
         {"Combatant", "Duelist", "Dragoon", "Warwitch", "Grand Warwitch", "Grand High Warwitch"}},

        {{"ranged", "nature magic"},
         {"Scout", "Forester", "Ranger", "Warder", "Arch Ward", "Supreme High Ward"},
         {"Scout", "Forester", "Ranger", "Wardess", "Arch Wardess", "Supreme High Wardess"}},

        {{"ranged", "combat magic"},
         {"Jager", "Conjurer", "Channeler", "Matross", "Master Matross", "Grandmaster Matross"}},

        {{"nature magic", "combat magic"},
         {"Acolyte", "Shaman", "Scholar", "Magus", "Grand Magus", "Grand High Magus"}},

        {{"melee"},
         {"Squire", "Soldier", "Warrior", "Knight", "Champion", "Grand Champion"}},

        {{"ranged"},
         {"Bowyer", "Archer", "Marksman", "Sharpshooter", "Master Sharpshooter", "Grandmaster Sharpshooter"}},

        {{"nature magic"},
         {"Apprentice", "Theurgist", "Magician", "Grand Mage", "Arch Mage", "Supreme Arch Mage"}},

        {{"combat magic"},
         {"Savant", "Hedge Wizard", "Wizard", "Sorcerer", "Grand Sorcerer", "Grand High Sorcerer"},
         {"Savant", "Hedge Wizard", "Wizard", "Sorceress", "Grand Sorceress", "Grand High Sorceress"}}
    };

    auto getBracket = [](float level) {
        if (level >= 100) return 5;
        if (level >= 50)  return 4;
        if (level >= 20)  return 3;
        if (level >= 11)  return 2;
        if (level >= 5)   return 1;
        if (level >= 1)   return 0;
        return -1;
    };

    int gender = 0; // TODO add gender getter GetGender(); // 0 = male, 1 = female

    for (const auto& group : titleGroups) {
        bool qualifies = true;
        int minBracket = 6;

        for (const string& skill : group.skills) {
            float level = Actor()->GetSkillLevel(skill.c_str());
            int bracket = getBracket(level);
            if (bracket == -1) {
                qualifies = false;
                break;
            }
            minBracket = min(minBracket, bracket);
        }

        if (qualifies) {
            if (gender == 1 && group.femaleTitles.size() == 6)
                return group.femaleTitles[minBracket];
            else
                return group.maleTitles[minBracket];
        }
    }

    return "Noob";
}



void Go :: AddChild (Go * child)
{
	if (child != NULL)
	{
		m_children.insert (child);
	}
}

GoBody * Go :: Body () const
{
	if (m_body != NULL)
	{
		return m_body;
	}
	
	throw logic_error ("null pointer referenced");
}

void Go :: ClearParent ()
{
	m_parent = NULL;
}

GoScriptComponent * Go :: GetComponent (const string & component) const
{
	map<string, GoScriptComponent *>::const_iterator iterator = m_scripts.find (component);
	if (iterator != m_scripts.end())
	{
		return iterator->second;
	}
	
	throw logic_error ("null pointer referenced");
}

bool Go :: HasActor () const
{
	return m_actor != NULL;
}

bool Go :: HasAspect () const
{
	return m_aspect != NULL;
}

bool Go :: HasAttack () const
{
	return m_aspect != NULL;
}

bool Go :: HasBody () const
{
	return m_body != NULL;
}

bool Go :: HasChild (Go * child) const
{
	GopSet::const_iterator iterator = m_children.find (child);
	return iterator != m_children.end();
}

bool Go :: HasCommon () const
{
	return m_common != NULL;
}

bool Go :: HasComponent (const string & component) const
{
	map<string, GoScriptComponent *>::const_iterator iterator = m_scripts.find (component);
	return iterator != m_scripts.end();
}

bool Go :: HasConversation () const
{
	return false;
}

bool Go :: HasDefend () const
{
	return m_defend != NULL;
}

bool Go :: HasGui () const
{
	return m_gui != NULL;
}

bool Go :: HasInventory () const
{
	return m_inventory != NULL;
}

bool Go :: HasMagic () const
{
	return false;
}

bool Go :: HasMind () const
{
	return m_mind != NULL;
}

bool Go :: HasPlacement () const
{
	return m_placement != NULL;
}

bool Go :: IsActor () const
{
	return m_actor != NULL && m_aspect != NULL && m_common != NULL && m_mind != NULL && m_placement != NULL;
}

bool Go :: IsArmor () const
{
	return IsItem() && m_defend != NULL;
	//return m_actor == NULL && m_aspect != NULL && m_common != NULL && m_defend != NULL && m_mind == NULL && m_placement != NULL;
}

bool Go :: IsBreakable () const
{
	return false;
}

bool Go :: IsContainer () const
{
	return false;
}

bool Go :: IsEquipped () const
{
	if (m_parent != NULL)
	{
		return m_parent->Inventory()->IsEquipped (this);
	}
	
	return false;
}

bool Go :: IsGhost () const
{
	return false;
}

bool Go :: IsInsideInventory () const
{
	if (m_parent != NULL)
	{
		return m_parent->Inventory()->Contains (this);
	}
	
	return false;
}

bool Go :: IsItem () const
{
	return m_actor == NULL && m_aspect != NULL && m_common != NULL && m_mind == NULL && m_placement != NULL;
}

bool Go :: IsMeleeWeapon () const
{
	if (m_attack != NULL)
	{
		return m_attack->IsMelee();
	}
	
	return false;
}

bool Go :: IsRangedWeapon () const
{
	if (m_attack != NULL)
	{
		return m_attack->IsRanged();
	}
	
	return false;
}

bool Go :: IsSpell () const
{
	return HasMagic();
}

bool Go :: IsSpellBook () const
{
	return IsItem() && m_inventory != NULL;
}

bool Go :: IsTeamMember (const Go * go) const
{
	return false;
}

bool Go :: IsWeapon () const
{
	return m_attack != NULL && IsItem();
	//return m_actor == NULL && m_aspect != NULL && m_attack != NULL && m_common != NULL && m_mind == NULL && m_placement != NULL;
}

void Go :: RemoveAllChildren ()
{
	m_children.clear();
}

void Go :: RemoveChild (Go * child)
{
	if (child != NULL)
	{
		m_children.erase (child);
	}
}

void Go :: Send (const WorldMessage & message)
{
	eWorldEvent event = message.WorldEvent();
	Go * from = message.SendFrom();

	/*if (from == this) {
		std::cerr << "[Go::Send] Warning: got frustum message from self for Go " << Goid() << " event: " << event << std::endl;
	}*/

	switch (event)
	{
		case we_entered_frustum:
		{
			if (from != NULL && from != this)
			{
				m_objects.insert (from);
			}
		}
		break;
		
		case we_left_frustum:
		{
			if (from != NULL && from != this)
			{
				m_objects.erase (from);
			}
		}
		break;
		
		default: break;
	}
	
	for (map<string, GoScriptComponent *>::iterator iterator = m_scripts.begin(); iterator != m_scripts.end(); iterator++)
	{
		GoScriptComponent * script = iterator->second;
		script->OnGoHandleMessage (message);
	}
}

void Go :: SetParent (Go * parent)
{
	if (parent != NULL)
	{
		m_parent = parent;
	}
}

GoActor * Go :: Actor () const
{
	if (m_actor != NULL)
	{
		return m_actor;
	}
	
	throw logic_error ("null pointer referenced");
}

GoAspect * Go :: Aspect () const
{
	if (m_aspect != NULL)
	{
		return m_aspect;
	}
	
	throw logic_error ("null pointer referenced");
}

GoAttack * Go :: Attack () const
{
	if (m_attack != NULL)
	{
		return m_attack;
	}
	
	throw logic_error ("null pointer referenced");
}

const GopSet & Go :: Children () const
{
	return m_children;
}

GoCommon * Go :: Common () const
{
	if (m_common != NULL)
	{
		return m_common;
	}
	
	throw logic_error ("null pointer referenced");
}

GoDefend* Go::Defend() const
{
	if (m_defend != NULL)
	{
		return m_defend;
	}

	throw logic_error("null pointer referenced");
}

GoGui* Go::Gui() const
{
	if (m_gui != NULL)
	{
		return m_gui;
	}

	throw logic_error("null pointer referenced");
}

uint32_t Go::Goid() const
{
	return m_goid;
}

uint32_t Go::Scid() const
{
	return m_scid;
}

GoInventory * Go :: Inventory () const
{
	if (m_inventory != NULL)
	{
		return m_inventory;
	}
	
	throw logic_error ("null pointer referenced");
}

eLifeState Go :: LifeState () const
{
	if (m_aspect != NULL)
	{
		return m_aspect->LifeState();
	}
	
	return ls_ignore;
}

GoMagic * Go :: Magic () const
{
	if (m_magic != NULL)
	{
		return m_magic;
	}

	throw logic_error ("null pointer referenced");
}

GoMind * Go :: Mind () const
{
	if (m_mind != NULL)
	{
		return m_mind;
	}
	
	throw logic_error ("null pointer referenced");
}

Go * Go :: Parent () const
{
	if (m_parent != NULL)
	{
		return m_parent;
	}
	
	throw logic_error ("null pointer referenced");
}

GoPlacement * Go :: Placement () const
{
	if (m_placement != NULL)
	{
		return m_placement;
	}
	
	throw logic_error ("null pointer referenced");
}

string Go :: TemplateName () const
{
	if (m_template_name.empty())
	{
		if (HasAspect() && (Aspect()->Model().empty() == false))
		{
			return Aspect()->Model();
		}
	}

	return m_template_name;
}

const GopSet & Go :: Frustum ()
{
	return m_objects;
}

void Go :: AddComponent (GoScriptComponent * component)
{
	if (component != NULL)
	{
		if (component->Name().empty() != true)
		{
			map<string, GoScriptComponent *>::iterator iterator = m_scripts.find (component->Name());
			if (iterator == m_scripts.end())
			{
				m_scripts[component->Name()] = component;
			}
		}
	}
}

void Go :: RemoveComponent (const string & component)
{
	map<string, GoScriptComponent *>::iterator iterator = m_scripts.find (component);
	if (iterator != m_scripts.end())
	{
		m_scripts.erase (iterator);
	}
}

void Go::SaveToXml(const string& folderName)
{
    const string idStr = to_string(Goid());
    const string path = "data/" + folderName + "/" + idStr + ".xml";

    xmlDoc* doc = xml::LoadFile(path);
    if (!doc) {
        cerr << "Failed to open " << path << ". Creating new xml" << endl;

		// Create the XML file for the region
		ofstream file(path);
		if (!file.is_open())
		{
			cerr << "Failed to create item XML file: " << path << endl;
			return;
		}
		file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		file << "<objects>\n";
		file << "<go id=\"" << idStr << "\">\n";
		file << "</go>\n";
		//file << "<go id=\"" << (size_t)10000 + g_engine.GetItems().size() << "\">\n";
		file << "</objects>\n";
		file.close();
        //return;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (!root || !xmlStrEqual(root->name, BAD_CAST "objects")) {
        cerr << "Invalid GO XML structure in " << path << endl;
        xmlFreeDoc(doc);
        return;
    }

    xmlNode* goNode = nullptr;
    for (xmlNode* node = root->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && xmlStrEqual(node->name, BAD_CAST "go")) {
            if (xml::XReadString(node, "id", "") == idStr) {
                goNode = node;
                break;
            }
        }
    }

    if (!goNode) {
        cerr << "[GO] Not found GO " << Goid() << " in " << path << endl;
        xmlFreeDoc(doc);
        return;
    }

    auto FindOrCreateChild = [](xmlNode* parent, const char* name) -> xmlNode* {
        for (xmlNode* child = parent->children; child; child = child->next)
            if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST name))
                return child;
        return nullptr;
    };

    if (HasActor())
        if (xmlNode* n = FindOrCreateChild(goNode, "actor"))
            Actor()->Save(n);

    if (HasAspect() && (folderName == "actors"))
        if (xmlNode* n = FindOrCreateChild(goNode, "aspect"))
            Aspect()->Save(n);

    if (HasAttack())
        if (xmlNode* n = FindOrCreateChild(goNode, "attack"))
            Attack()->Save(n);

    if (HasCommon())
        if (xmlNode* n = FindOrCreateChild(goNode, "common"))
            Common()->Save(n);

    if (HasInventory())
        if (xmlNode* n = FindOrCreateChild(goNode, "inventory"))
            Inventory()->Save(n);

    if (HasPlacement())
        if (xmlNode* n = FindOrCreateChild(goNode, "placement"))
            Placement()->Save(n);

    if (!xml::SaveFile(doc, path))
        cerr << "Failed to save XML to " << path << endl;

    xmlFreeDoc(doc);
}

void Go :: CalculateStatus()
{
	//cout << "Entering Calc Status" << endl;
	float hpAmount = 0;
	float mpAmount = 0;
	if (Actor()->CanLevelUp())
	{
		// Max HP/MP & Recovery Unit
		float strLevel = (floor(Actor()->GetSkillLevel("strength")) + 1.0f);
		float dexLevel = (floor(Actor()->GetSkillLevel("dexterity")) + 1.0f);
		float intLevel = (floor(Actor()->GetSkillLevel("intelligence")) + 1.0f);
		float strHpAmount = strLevel * 29.4f;
		float dexHpAmount = dexLevel * 9.8f;
		float intHpAmount = intLevel * 9.8f;
		hpAmount = strHpAmount + dexHpAmount + intHpAmount;
		//cout << "Calculating status for GO: " << Goid() << " total amount: " << hpAmount << endl;
		Aspect()->SetMaxLife(hpAmount);
		Aspect()->SetHpRecUnit(strLevel);

		float strMpAmount = strLevel;
		float dexMpAmount = dexLevel * 4.0f;
		float intMpAmount = intLevel * 25.0f;
		mpAmount = strMpAmount + dexMpAmount + intMpAmount;
		Aspect()->SetMaxMana(mpAmount);
		Aspect()->SetMpRecUnit(intLevel);

		// min/max dmg
		float minDmg = 2.0f + ((strLevel -1.0f) * 1.4f);
		float maxDmg = 4.0f + ((strLevel -1.0f) * 1.6f);
		//cout << "strLevel: " << strLevel << " damage min: " << minDmg << " damage max: " << maxDmg << endl;
		Attack()->SetDamageMin(minDmg);
		Attack()->SetDamageMax(maxDmg);
	}
	else
	{
		//cout << "Calculating status for GO: " << Goid() << " cant level up" << endl;
		// if NPC set hardcoded maxlife val
		// maxLife = maxLife + totalAmounts;
		//float maxLife = godb.GetTemplate(Aspect()->GetModelName())->Aspect()->MaxLife();
	}
}

/*
int Go :: GetComponentInt (const string & component, const string & property)
{
	map<string, GoScriptComponent *>::iterator iterator = m_scripts.find (component);
	if (iterator != m_scripts.end())
	{
		GoScriptComponent * script = iterator->second;
		return script->GetComponentInt (property);
	}

	return 0;
}

float Go :: GetComponentFloat (const string & component, const string & property)
{
	map<string, GoScriptComponent *>::iterator iterator = m_scripts.find (component);
	if (iterator != m_scripts.end())
	{
		GoScriptComponent * script = iterator->second;
		return script->GetComponentFloat (property);
	}

	return 0.0f;
}

bool Go :: GetComponentBool (const string & component, const string & property)
{
	map<string, GoScriptComponent *>::iterator iterator = m_scripts.find (component);
	if (iterator != m_scripts.end())
	{
		GoScriptComponent * script = iterator->second;
		return script->GetComponentBool (property);
	}

	return false;
}

string Go :: GetComponentString (const string & component, const string & property)
{
	map<string, GoScriptComponent *>::iterator iterator = m_scripts.find (component);
	if (iterator != m_scripts.end())
	{
		GoScriptComponent * script = iterator->second;
		return script->GetComponentString (property);
	}

	return "";
}

void Go :: SetComponentInt (const string & component, const string & property, int value)
{
	map<string, GoScriptComponent*>::iterator iterator = m_scripts.find (component);
	if (iterator != m_scripts.end())
	{
		GoScriptComponent * script = iterator->second;
		script->SetComponentInt (property, value);
	}
}

void Go :: SetComponentFloat (const string & component, const string & property, float value)
{
	map<string, GoScriptComponent *>::iterator iterator = m_scripts.find (component);
	if (iterator != m_scripts.end())
	{
		GoScriptComponent * script = iterator->second;
		script->SetComponentFloat (property, value);
	}
}

void Go :: SetComponentBool (const string & component, const string & property, bool value)
{
	map<string, GoScriptComponent *>::iterator iterator = m_scripts.find (component);
	if (iterator != m_scripts.end())
	{
		GoScriptComponent * script = iterator->second;
		script->SetComponentBool (property, value);
	}
}

void Go :: SetComponentString (const string & component, const string & property, const string & value)
{
	map<string, GoScriptComponent *>::iterator iterator = m_scripts.find (component);
	if (iterator != m_scripts.end())
	{
		GoScriptComponent * script = iterator->second;
		script->SetComponentString (property, value);
	}
}
*/
