/*
 * GoMagic.cpp
 *
 *  Created on: 25.05.2025
 *      Author: Cryden
 */
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
#include "GoMagic.hpp"
#include "../enum/eMagicClass.hpp"
#include "../helper/Helper.h"

GoMagic :: GoMagic (Go * go) : GoComponent (go)
{
}

GoMagic::GoMagic(Go* newGo, const GoMagic& other) : GoComponent(newGo) // attach to new Go
{
	m_skill_class = other.m_skill_class;       // default "mc_none"
	m_required_level = other.m_required_level;    // default 1.0f
	m_cast_sub_animation = other.m_cast_sub_animation; // default 1
	m_cast_range = other.m_cast_range;        // default 1.0f
	m_cast_reload_delay = other.m_cast_reload_delay;  // default 1
	m_effect_duration = other.m_effect_duration;   // default 1
	m_defensive = other.m_defensive;         // default false
	m_offensive = other.m_offensive;         // default false
}

GoMagic :: GoMagic (Go * go, xmlNode * node) : GoComponent (go)
{
	if (node != NULL)
	{
		xmlNode * current = NULL;
		for (current = node->children; current != NULL; current = current->next)
		{
			if (current->type != XML_ELEMENT_NODE) continue;

			if (xmlStrEqual (current->name, (const xmlChar *) "spell_class") != 0)
			{
				m_skill_class = xml::XReadString(current, "value", "mc_none");
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "required_level") != 0)
			{
				m_required_level = xml::ReadAttribute<float> (current, "value", 1.0);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "cast_sub_animation") != 0)
			{
				m_cast_sub_animation = xml::ReadAttribute<uint8_t> (current, "value", 1);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "cast_range") != 0)
			{
				m_cast_range = xml::ReadAttribute<float> (current, "value", 1.0);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "cast_reload_delay") != 0)
			{
				m_cast_reload_delay = xml::ReadAttribute<int32_t> (current, "value", 1);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "effect_duration") != 0)
			{
				m_effect_duration = xml::ReadAttribute<int32_t> (current, "value", 1);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "defensive") != 0)
			{
				m_defensive = xml::ReadAttribute<bool> (current, "value", false);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "offensive") != 0)
			{
				m_offensive = xml::ReadAttribute<bool> (current, "value", false);
			}
		}
	}
}

GoMagic::GoMagic(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
	if (tmplComp == nullptr)
		return;

	const string* f;
	if (f = tmplComp->GetField("flesh")) { try { m_skill_class = *f; } catch (...) { m_skill_class = "mc_none"; } }
	if (f = tmplComp->GetField("required_level")) { try { m_required_level = std::stof(*f); } catch (...) { m_required_level = 1.0f; } }
	if (f = tmplComp->GetField("cast_sub_animation")) { try { m_cast_sub_animation = static_cast<int8_t>(std::stoi(*f)); } catch (...) { m_cast_sub_animation = 1; } }
	if (f = tmplComp->GetField("cast_range")) { try { m_cast_range = std::stof(*f); } catch (...) { m_cast_range = 1.0f; } }
	if (f = tmplComp->GetField("cast_reload_delay")) { try { m_cast_reload_delay = static_cast<int32_t>(std::stoi(*f)); } catch (...) { m_cast_reload_delay = 1; } }
	if (f = tmplComp->GetField("effect_duration")) { try { m_effect_duration = static_cast<int32_t>(std::stoi(*f)); } catch (...) { m_effect_duration = 1; } }
	if (f = tmplComp->GetField("defensive")) { if (FromString(*f, m_defensive) != true) m_defensive = false; }
	if (f = tmplComp->GetField("offensive")) { if (FromString(*f, m_offensive) != true) m_offensive = false; }
}

void GoMagic::InheritFrom(const GoMagic& other)
{
	if (m_skill_class == "mc_none")          m_skill_class = other.m_skill_class;
	if (m_required_level == 1.0f)           m_required_level = other.m_required_level;
	if (m_cast_sub_animation == 1)           m_cast_sub_animation = other.m_cast_sub_animation;
	if (m_cast_range == 1.0f)               m_cast_range = other.m_cast_range;
	if (m_cast_reload_delay == 1)            m_cast_reload_delay = other.m_cast_reload_delay;
	if (m_effect_duration == 1)              m_effect_duration = other.m_effect_duration;
	if (m_defensive == false)                m_defensive = other.m_defensive;
	if (m_offensive == false)                m_offensive = other.m_offensive;
}

bool GoMagic :: IsCastableOn (Go * go) const
{
	return true; // TODO is castable
}

bool GoMagic :: IsDefensive () const
{
	return m_defensive;
}

bool GoMagic :: IsOffensive () const
{
	return m_offensive;
}

bool GoMagic :: Cast (Go * go)
{
	return false; // TODO cast the spell
}

float GoMagic :: CastRange () const
{
	return m_cast_range;
}

int32_t GoMagic :: CastReloadDelay () const
{
	return m_cast_reload_delay;
}

uint8_t GoMagic :: CastSubAnimation () const
{
	return m_cast_sub_animation;
}

int32_t GoMagic :: EffectDuration () const
{
	return m_effect_duration;
}

string GoMagic :: SkillClass () const
{
	return m_skill_class;
}



