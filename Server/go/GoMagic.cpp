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

GoMagic::GoMagic(Go* go, const std::map<std::string, std::string>& r) : GoComponent(go)
{
	m_skill_class = r.at("spell_class");
	m_required_level = std::stof(r.at("required_level"));
	m_cast_sub_animation = static_cast<uint8_t>(std::stoi(r.at("cast_sub_animation")));
	m_cast_range = std::stof(r.at("cast_range"));
	m_cast_reload_delay = std::stoi(r.at("cast_reload_delay"));
	m_effect_duration = std::stoi(r.at("effect_duration"));
	m_defensive = r.at("is_defensive") == "1";
	m_offensive = r.at("is_offensive") == "1";
}

void GoMagic::Save(MySQL& db)
{
	std::string q =
		"INSERT INTO t_go_magic (go_id, spell_class, required_level, cast_sub_animation, "
		"cast_range, cast_reload_delay, effect_duration, is_defensive, is_offensive) VALUES (" +
		std::to_string(GetGo()->Goid()) + ", '" +
		m_skill_class + "', " +
		std::to_string(m_required_level) + ", " +
		std::to_string(m_cast_sub_animation) + ", " +
		std::to_string(m_cast_range) + ", " +
		std::to_string(m_cast_reload_delay) + ", " +
		std::to_string(m_effect_duration) + ", " +
		std::to_string(m_defensive ? 1 : 0) + ", " +
		std::to_string(m_offensive ? 1 : 0) + ") "
		"ON DUPLICATE KEY UPDATE "
		"spell_class=VALUES(spell_class), "
		"required_level=VALUES(required_level), "
		"cast_sub_animation=VALUES(cast_sub_animation), "
		"cast_range=VALUES(cast_range), "
		"cast_reload_delay=VALUES(cast_reload_delay), "
		"effect_duration=VALUES(effect_duration), "
		"is_defensive=VALUES(is_defensive), "
		"is_offensive=VALUES(is_offensive)";

	db.AsyncQuery(q, [](const auto&) {});
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



