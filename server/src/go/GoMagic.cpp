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

#include "Go.hpp"
#include "GoMagic.hpp"

GoMagic :: GoMagic (Go * go) : GoComponent (go)
{
}

GoMagic :: GoMagic (Go * go, xmlNode * node) : GoComponent (go)
{
	if (node != NULL)
	{
		xmlNode * current = NULL;
		for (current = node->children; current != NULL; current = current->next)
		{
			if (current->type != XML_ELEMENT_NODE) continue;

			if (xmlStrEqual (current->name, (const xmlChar *) "skill_class") != 0)
			{
				m_skill_class = xml::XReadString (current, "value", "");
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "required_level") != 0)
			{
				m_required_level = xml::ReadAttribute<float> (current, "value", 1.0);
			}
			else if (xmlStrEqual (current->name, (const xmlChar *) "cast_sub_animation") != 0)
			{
				m_cast_sub_animation = xml::ReadAttribute<u_int8_t> (current, "value", 1);
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

u_int8_t GoMagic :: CastSubAnimation () const
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



