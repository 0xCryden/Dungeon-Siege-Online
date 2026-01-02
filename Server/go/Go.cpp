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
Go::Go(const TemplateData& tmpl, uint32_t goid, const string& pcontent)
	: m_template_name(tmpl.name), m_pcontent_query(pcontent), m_specializes(tmpl.specializes),
	m_parent(NULL), m_actor(NULL), m_aspect(NULL), m_attack(NULL),
	m_body(NULL), m_common(NULL), m_defend(NULL), m_gui(NULL),
	m_inventory(NULL), m_magic(NULL), m_mind(NULL), m_placement(NULL), m_conversation(NULL)
{
	if (goid != 0)
		m_goid = goid;

	if (tmpl.scid != "")
		m_scid = static_cast<uint32_t>(std::stoul(tmpl.scid, nullptr, 16));

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
	if ((comp = tmpl.GetComponent("conversation"))) { m_conversation = new GoConversation(this, comp); }
}

// instantiate from contentdb
Go :: Go (const Go * go, uint32_t goid, const string& pcontent_query) : m_specializes(""), m_pcontent_query(pcontent_query), m_parent(NULL), m_actor(NULL), m_aspect(NULL), m_attack(NULL), m_body(NULL), m_common(NULL), m_defend(NULL), m_gui(NULL), m_inventory(NULL), m_magic(NULL), m_mind(NULL), m_placement(NULL), m_conversation(NULL)
{
	m_goid = goid;
	m_scid = 0;
	m_template_name = go->m_template_name;
	if (pcontent_query == "")
		m_pcontent_query = go->m_pcontent_query;

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

	if (go->m_placement != NULL) m_placement = new GoPlacement(this);
	if (go->m_conversation != NULL) m_conversation = new GoConversation(this);
}

Go :: ~Go ()
{
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
	if (m_conversation) delete m_conversation;
	
	map<string, GoScriptComponent *>::iterator iterator = m_scripts.begin();
	while (iterator != m_scripts.end())
	{
		delete iterator->second;
	}
}

void Go::LoadFromDatabase(MySQL& db, std::function<void(Go*)> onLoaded)
{
	std::string query =
		"SELECT component_type FROM t_go_components WHERE go_id = " +
		std::to_string(m_goid);

	cout << "Loading Goid " << std::to_string(m_goid) << endl;

	db.AsyncQuery(query,
		[this, dbPtr = &db, onLoaded = std::move(onLoaded)](const auto& rows)
		{
			auto pending = std::make_shared<std::atomic<int>>(rows.size());

			// Lambda to call when one component is done
			auto doneOne = [pending, this, onLoaded]()
				{
					if (pending->fetch_sub(1) == 1)
					{
						// Queue final callback to main thread
						g_engine.m_mainThreadJobs.push([this, onLoaded]()
							{
								onLoaded(this);
							});
					}
				};

			if (rows.empty())
			{
				// No components  immediately push to main thread
				g_engine.m_mainThreadJobs.push([this, onLoaded]()
					{
						onLoaded(this);
					});
				return;
			}

			for (const auto& row : rows)
			{
				const std::string& type = row.at("component_type");
				if (type == "actor")
					LoadActor(*dbPtr, doneOne);
				else if (type == "aspect")
					LoadAspect(*dbPtr, doneOne);
				else if (type == "placement")
					LoadPlacement(*dbPtr, doneOne);
				else if (type == "common")
					LoadCommon(*dbPtr, doneOne);
				else if (type == "inventory")
					LoadInventory(*dbPtr, doneOne);
				else if (type == "attack")
					LoadAttack(*dbPtr, doneOne);
				else if (type == "body")
					LoadBody(*dbPtr, doneOne);
				else if (type == "defend")
					LoadDefend(*dbPtr, doneOne);
				else if (type == "gui")
					LoadGui(*dbPtr, doneOne);
				else if (type == "magic")
					LoadMagic(*dbPtr, doneOne);
				else if (type == "mind")
					LoadMind(*dbPtr, doneOne);
				else
					doneOne(); // unknown component
			}
		}
	);
}

void Go::LoadAttack(MySQL& db, std::function<void()> done)
{
	std::string query =
		"SELECT * FROM t_go_attack WHERE go_id = " + std::to_string(m_goid);

	db.AsyncQuery(query,
		[this, done](const auto& rows)
		{
			if (!rows.empty())
			{
				m_attack = new GoAttack(this, rows[0]);
			}
			done();
		}
	);
}
void Go::LoadDefend(MySQL& db, std::function<void()> done)
{
	std::string query =
		"SELECT * FROM t_go_defend WHERE go_id = " + std::to_string(m_goid);

	db.AsyncQuery(query,
		[this, done](const auto& rows)
		{
			if (!rows.empty())
			{
				m_defend = new GoDefend(this, rows[0]);
			}
			done();
		}
	);
}
void Go::LoadBody(MySQL& db, std::function<void()> done)
{
	std::string query =
		"SELECT * FROM t_go_body WHERE go_id = " + std::to_string(m_goid);

	db.AsyncQuery(query,
		[this, done](const auto& rows)
		{
			if (!rows.empty())
			{
				m_body = new GoBody(this, rows[0]);
			}
			done();
		}
	);
}
void Go::LoadGui(MySQL& db, std::function<void()> done)
{
	std::string query =
		"SELECT * FROM t_go_gui WHERE go_id = " + std::to_string(m_goid);

	db.AsyncQuery(query,
		[this, done](const auto& rows)
		{
			if (!rows.empty())
			{
				m_gui = new GoGui(this, rows[0]);
			}
			done();
		}
	);
}
void Go::LoadMagic(MySQL& db, std::function<void()> done)
{
	std::string query =
		"SELECT * FROM t_go_magic WHERE go_id = " + std::to_string(m_goid);

	db.AsyncQuery(query,
		[this, done](const auto& rows)
		{
			if (!rows.empty())
			{
				m_magic = new GoMagic(this, rows[0]);
			}
			done();
		}
	);
}
void Go::LoadMind(MySQL& db, std::function<void()> done)
{
	std::string query =
		"SELECT * FROM t_go_mind WHERE go_id = " + std::to_string(m_goid);

	db.AsyncQuery(query,
		[this, done](const auto& rows)
		{
			if (!rows.empty())
			{
				m_mind = new GoMind(this, rows[0]);
			}
			done();
		}
	);
}

void Go::LoadPlacement(MySQL& db, std::function<void()> done)
{
	std::string query =
		"SELECT region, node_id, pos_x, pos_y, pos_z "
		"FROM t_go_placement WHERE go_id = " + std::to_string(m_goid);

	db.AsyncQuery(query,
		[this, done](const auto& rows)
		{
			if (!rows.empty())
			{
				m_placement = new GoPlacement(this, rows[0]);
			}

			done();
		}
	);
}

void Go::LoadActor(MySQL& db, std::function<void()> done)
{
	std::string query =
		"SELECT * FROM t_go_actor WHERE go_id = " + std::to_string(m_goid);

	db.AsyncQuery(query,
		[this, done](const auto& rows)
		{
			if (!rows.empty())
			{
				m_actor = new GoActor(this, rows[0]);
			}
			done();
		}
	);
}

void Go::LoadAspect(MySQL& db, std::function<void()> done)
{
	std::string query =
		"SELECT * FROM t_go_aspect WHERE go_id = " + std::to_string(m_goid);

	db.AsyncQuery(query,
		[this, done](const auto& rows)
		{
			if (!rows.empty())
			{
				m_aspect = new GoAspect(this, rows[0]);
			}
			done();
		}
	);
}

void Go::LoadCommon(MySQL& db, std::function<void()> done)
{
	std::string query =
		"SELECT * FROM t_go_common WHERE go_id = " + std::to_string(m_goid);

	db.AsyncQuery(query,
		[this, done](const auto& rows)
		{
			if (!rows.empty())
			{
				m_common = new GoCommon(this, rows[0]);
			}
			done();
		}
	);
}

void Go::LoadInventory(MySQL& db, std::function<void()> done)
{
	std::string invQuery =
		"SELECT * FROM t_go_inventory WHERE go_id = " + std::to_string(m_goid) + ";";

	db.AsyncQuery(invQuery,
		[this, &db, done](const auto& invRows)
		{
			if (invRows.empty())
			{
				done();
				return;
			}

			// Create inventory from base row
			m_inventory = new GoInventory(this, invRows[0]);

			// Second query: items
			std::string itemsQuery =
				"SELECT * FROM t_go_inventory_items WHERE go_id = " +
				std::to_string(m_goid) + ";";

			db.AsyncQuery(itemsQuery,
				[this, done](const auto& itemRows)
				{
					for (const auto& row : itemRows)
					{
						// convert row to item
						const string& equipSlot = row.at("equip_slot");

						Go* item = godb.FindGoById(stoi(row.at("item_go_id")));
						if (!item) // error template not found
						{
							cout << "ERROR item not found for inventory of go " << m_goid << endl;
							continue;
						}

						if (m_inventory->Add(item))
						{
							// Set inventory location
							item->SetLoc(item->IntendedLoc());

							// If equip_slot is present, equip it
							eEquipSlot slot = StringToNum(equipSlot); // You need to implement this
							if (slot != es_none)
							{
								m_inventory->Equip(slot, item); // Will only succeed if not already equipped
							}
						}
					}

					done();
				}
			);
		}
	);
}

double Go :: GetDistanceTo(Go * target) const
{
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

		bool isPcontentQuery = false;
		string pContent;
		if (templateName[0] == '#')
		{
			size_t colonPos = templateName.find(':');

			if (colonPos != string::npos && colonPos > 1 && colonPos < templateName.size() - 1)
			{
				pContent = templateName; // pContent = templateName.substr(colonPos + 1);
				templateName = templateName.substr(1, colonPos - 1); // skip '#'
				isPcontentQuery = true;
			}
			else
			{
				cout << "[error] Invalid format. Expected #<template>:<value>\n";
				return;
			}
		}
		else
		{
			pContent = "#" + templateName;
		}
		
		/*if (TemplateData* tpl = manager.GetTemplate(templateName))
		{
			cout << "[info] template: " << tpl->name << "\n";
			if (!tpl->specializes.empty())
				cout << "  specializes: " << tpl->specializes << "\n";

			for (const auto& [compname, comp] : tpl->components) {
				gas.LogComponent(compname, comp, "  ");
			}
		}*/

        // Try to spawn or add using the template name
		godb.SpawnGo(templateName, this, pContent);
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

            	Actor()->SetSkillLevel("uber", levels * 3);

            	Actor()->SetSkillLevel("strength", levels);
            	Actor()->SetSkillLevel("dexterity", levels);
            	Actor()->SetSkillLevel("intelligence", levels);

            	Actor()->SetSkillLevel("melee", levels);
            	Actor()->SetSkillLevel("ranged", levels);
            	Actor()->SetSkillLevel("nature magic", levels);
            	Actor()->SetSkillLevel("combat magic", levels);
            }
            else // specific skill
            {
            	if (!Actor()->HasSkill(skillName))
            		return;

				if (skillName == "uber")
				{
					cout << "Cant level uber level. Please level up attribute instead" << endl;
					return;
				}

            	float levels = stof(levelAmount);
            	Actor()->SetSkillLevel(skillName, levels);

				if (skillName == "strength")
				{
					Actor()->SetSkillLevel("uber", levels + Actor()->GetSkillLevel("intelligence") + Actor()->GetSkillLevel("dexterity"));
				}
				else if (skillName == "intelligence")
				{
					Actor()->SetSkillLevel("uber", levels + Actor()->GetSkillLevel("strength") + Actor()->GetSkillLevel("dexterity"));
				}
				else if (skillName == "dexterity")
				{
					Actor()->SetSkillLevel("uber", levels + Actor()->GetSkillLevel("strength") + Actor()->GetSkillLevel("intelligence"));
				}
            }
			CalculateStatus();
			Aspect()->SetCurrentLife(Aspect()->MaxLife());
			Aspect()->SetCurrentMana(Aspect()->MaxMana());

			g_engine.UpdateGo(this, we_goupdate_lifestate);
			g_engine.UpdateGo(this, we_goupdate_skills);
        }
    }
    else
    {
    	cout << "Unknown command: " << cmd << endl;
    }
}

eInventoryLocation Go :: IntendedLoc() const
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
	return m_attack != NULL;
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
	return m_conversation != NULL;
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
	return m_magic != NULL;
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

void Go::CopyPlacement(GoPlacement* placement)
{
	delete m_placement;
	m_placement = new GoPlacement(this, *placement); // or new GoActor(actor)
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

GoPlacement* Go::Placement() const
{
	if (m_placement != NULL)
	{
		return m_placement;
	}

	throw logic_error("null pointer referenced");
}

GoConversation* Go::Conversation() const
{
	if (m_conversation != NULL)
	{
		return m_conversation;
	}

	throw logic_error("null pointer referenced");
}

string Go::TemplateName() const
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

string Go::pContentQuery() const
{
	if (m_pcontent_query.empty())
	{
		return "";
	}

	return m_pcontent_query;
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

void Go::Save(MySQL& db)
{
	cout << "Saving GoID: " << Goid() << endl;

	// 0) ENSURE parent row exists
	std::string ensureGo =
		"INSERT INTO t_gos (go_id, template_name, pcontent_query) VALUES (" +
		std::to_string(m_goid) + ", '" +
		m_template_name + "', '" +
		m_pcontent_query + "' " +
		") ON DUPLICATE KEY UPDATE go_id = go_id";

	db.AsyncQuery(ensureGo,
		[this, &db](const auto&)
		{
			// 1) Delete old components AFTER parent exists
			std::string del =
				"DELETE FROM t_go_components WHERE go_id = " +
				std::to_string(m_goid);

			db.AsyncQuery(del,
				[this, &db](const auto&)
				{
					// 2) Insert components AFTER delete
					auto insertComp = [&](const char* type)
						{
							std::string q =
								"INSERT INTO t_go_components (go_id, component_type) VALUES (" +
								std::to_string(m_goid) + ", '" + type + "')";

							db.AsyncQuery(q, [](const auto&) {});
						};

					if (HasActor()) { insertComp("actor");      Actor()->Save(db); }
					if (HasAspect()) { insertComp("aspect");     Aspect()->Save(db); }
					if (HasAttack()) { insertComp("attack");     Attack()->Save(db); }
					if (HasBody()) { insertComp("body");       Body()->Save(db); }
					if (HasCommon()) { insertComp("common");     Common()->Save(db); }
					if (HasDefend()) { insertComp("defend");     Defend()->Save(db); }
					if (HasGui()) { insertComp("gui");        Gui()->Save(db); }
					if (HasInventory()) { insertComp("inventory"); Inventory()->Save(db); }
					if (HasMagic()) { insertComp("magic");      Magic()->Save(db); }
					if (HasMind()) { insertComp("mind");       Mind()->Save(db); }
					if (HasPlacement()) { insertComp("placement"); Placement()->Save(db); }
				});
		});
}

void Go :: CalculateStatus()
{
	cout << "Entering Calc Status for " << to_string(Goid()) << endl;
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
