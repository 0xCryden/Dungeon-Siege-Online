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
#include "GoAspect.hpp"
#include "../events/SendWorldMessageEvent.hpp"
#include <cmath>
#include "../helper/Helper.h"
#include "../Engine.hpp"

GoAspect :: GoAspect (Go * go) : GoComponent (go)
{
	//if (!m_render_scale.has_value())
	m_render_scale = 1.0f;
}

GoAspect::GoAspect(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
	if (tmplComp == nullptr)
		return;

	if (!m_render_scale.has_value())
		m_render_scale = 1.0f;

	//m_life_recovery_period = 4;
	//m_mana_recovery_period = 4;

	const string* f;
	if (f = tmplComp->GetField("bounding_sphere_radius")) { try { m_bounding_sphere_radius = std::stof(*f); } catch (...) { m_bounding_sphere_radius = 0.0f; } }

	if (f = tmplComp->GetField("max_life")) { try { m_max_life = std::stof(*f); } catch (...) { m_max_life = 0.0f; } }
	if (f = tmplComp->GetField("max_mana")) { try { m_max_mana = std::stof(*f); } catch (...) { m_max_mana = 0.0f; } }

	m_current_life = m_max_life;
	m_current_mana = m_max_mana;

	if (f = tmplComp->GetField("current_life")) { try { m_current_life = std::stof(*f); } catch (...) { m_current_life = 0.0f; } }
	if (f = tmplComp->GetField("current_mana")) { try { m_current_mana = std::stof(*f); } catch (...) { m_current_mana = 0.0f; } }

	if (f = tmplComp->GetField("flesh")) { try { m_textures[0] = *f; } catch (...) { m_textures[0] = ""; } }
	if (f = tmplComp->GetField("cloth")) { try { m_textures[1] = *f; } catch (...) { m_textures[1] = ""; } }

	if (f = tmplComp->GetField("is_invincible")) { if (FromString(*f, m_invincible) != true) m_invincible = false; }
	if (f = tmplComp->GetField("is_visible")) { if (FromString(*f, m_visible) != true) m_visible = true; }


	if (f = tmplComp->GetField("life_recovery_period")) { try { m_life_recovery_period = static_cast<int16_t>(std::stof(*f)); } catch (...) { m_life_recovery_period = 0; } }
	if (f = tmplComp->GetField("life_recovery_unit")) { try { m_life_recovery_unit = std::stof(*f); } catch (...) { m_life_recovery_unit = 0.0f; } }
	if (f = tmplComp->GetField("life_state")) { if (FromString(*f, m_life_state) != true) m_life_state = ls_alive_conscious; }
	if (f = tmplComp->GetField("last_died")) { try { m_last_died = static_cast<int64_t>(std::stof(*f)); } catch (...) { m_last_died = 0; } }
	if (f = tmplComp->GetField("mana_recovery_period")) { try { m_mana_recovery_period = static_cast<int16_t>(std::stoi(*f)); } catch (...) { m_mana_recovery_period = 0; } }
	if (f = tmplComp->GetField("mana_recovery_unit")) { try { m_mana_recovery_unit = std::stof(*f); } catch (...) { m_mana_recovery_unit = 0.0f; } }
	if (f = tmplComp->GetField("model")) { try { m_model = *f; } catch (...) { m_model = ""; } }
	if (f = tmplComp->GetField("render_scale")) { try { m_render_scale = std::stof(*f); } catch (...) { m_render_scale = 1.0f; } }
	if (f = tmplComp->GetField("experience_value")) { try { m_experience_value = std::stof(*f); } catch (...) { m_experience_value = 1.0f; } }
}

GoAspect::GoAspect(Go* go, const std::map<std::string, std::string>& r) : GoComponent(go)
{
	m_bounding_sphere_radius = std::stof(r.at("bounding_sphere_radius"));
	m_current_life = std::stof(r.at("current_life"));
	m_max_life = std::stof(r.at("max_life"));
	m_current_mana = std::stof(r.at("current_mana"));
	m_max_mana = std::stof(r.at("max_mana"));

	m_textures[0] = r.at("flesh");
	m_textures[1] = r.at("cloth");

	m_invincible = r.at("is_invincible") == "1";
	m_visible = r.at("is_visible") == "1";
	string lsStr = r.at("life_state");
	if (FromString(lsStr, m_life_state) != true) { m_life_state = ls_alive_conscious; }

	m_last_died = std::stoi(r.at("last_died"));
	m_model = r.at("model");
	m_render_scale = std::stof(r.at("render_scale"));
	m_experience_value = std::stof(r.at("experience_value"));
}

void GoAspect::Save(MySQL& db)
{
	std::string q =
		"INSERT INTO t_go_aspect (go_id, bounding_sphere_radius, current_life, max_life, "
		"current_mana, max_mana, flesh, cloth, is_invincible, is_visible, "
		"life_state, last_died, model, render_scale, experience_value) VALUES (" +
		std::to_string(GetGo()->Goid()) + ", " +
		std::to_string(m_bounding_sphere_radius) + ", " +
		std::to_string(m_current_life) + ", " +
		std::to_string(m_max_life) + ", " +
		std::to_string(m_current_mana) + ", " +
		std::to_string(m_max_mana) + ", '" +
		m_textures[0] + "', '" +
		m_textures[1] + "', " +
		std::to_string(m_invincible ? 1 : 0) + ", " +
		std::to_string(m_visible ? 1 : 0) + ", '" +
		ToString(m_life_state) + "', " +
		std::to_string(m_last_died) + ", '" +
		m_model + "', " +
		std::to_string(m_render_scale.value()) + ", " +
		std::to_string(m_experience_value) + ") "
		"ON DUPLICATE KEY UPDATE "
		"bounding_sphere_radius=VALUES(bounding_sphere_radius), "
		"current_life=VALUES(current_life), max_life=VALUES(max_life), "
		"current_mana=VALUES(current_mana), max_mana=VALUES(max_mana), "
		"flesh=VALUES(flesh), cloth=VALUES(cloth), "
		"is_invincible=VALUES(is_invincible), is_visible=VALUES(is_visible), "
		"life_state=VALUES(life_state), last_died=VALUES(last_died), "
		"model=VALUES(model), render_scale=VALUES(render_scale), "
		"experience_value=VALUES(experience_value)";

	db.AsyncQuery(q, [](const auto&) {});
}

float GoAspect :: BoundingSphereRadius () const
{
	return m_bounding_sphere_radius;
}

float GoAspect :: CurrentLife () const
{
	return m_current_life;
}

float GoAspect :: CurrentMana () const
{
	return m_current_mana;
}

string GoAspect :: GetDynamicTexture (int index)
{
	if (index == 0 || index == 1)
	{
		return m_textures[index];
	}
	
	return "";
}

string GoAspect :: GetModelName ()
{
	return m_model;
}

bool GoAspect :: IsInvincible () const
{
	return m_invincible;
}

bool GoAspect :: IsVisible () const
{
	return m_visible;
}

int16_t GoAspect :: LifeRecoveryPeriod () const
{
	return m_life_recovery_period;
}

float GoAspect :: LifeRecoveryUnit () const
{
	return m_life_recovery_unit;
}

eLifeState GoAspect :: LifeState () const
{
	return m_life_state;
}

int64_t GoAspect :: LastDied () const
{
	return m_last_died;
}

int16_t GoAspect :: ManaRecoveryPeriod () const
{
	return m_mana_recovery_period;
}

float GoAspect :: ManaRecoveryUnit () const
{
	return m_mana_recovery_unit;
}

float GoAspect :: MaxLife () const
{
	return m_max_life;
}

float GoAspect :: MaxMana () const
{
	return m_max_mana;
}

string GoAspect :: Model () const
{
	return m_model;
}

float GoAspect :: RenderScale () const
{
	return m_render_scale.value();
}

void GoAspect :: SetCurrentLife (float life)
{
	m_current_life = life;
}

void GoAspect :: SetCurrentMana (float mana)
{
	m_current_mana = mana;
}

void GoAspect :: SetDynamicTexture (int index, const string & texture)
{
	if (index == 0 || index == 1)
	{
		m_textures[index] = texture;
	}
}

void GoAspect :: SetIsVisible (bool visible)
{
	m_visible = visible;
}

void GoAspect :: SetLifeState (eLifeState state)
{
	m_life_state = state;

	if (state == ls_dead_normal)
	{
		m_last_died = CurrentTime();
		// TODO add auto respawn 60 second timer
		//PostWorldMessage(we_resurrected, GetGo(), GetGo(), "", 60000);
	}

	//g_engine.UpdateGo(GetGo(), we_goupdate_lifestate);
}

void GoAspect :: SetMaxLife (float life)
{
	m_max_life = life;
}

void GoAspect :: SetMaxMana (float mana)
{
	m_max_mana = mana;
}

void GoAspect :: SetModel (const string & model)
{
	m_model = model;
}

void GoAspect :: SetRenderScale (float scale)
{
	m_render_scale = scale;
}

void GoAspect::RecoverLife()
{
	if (m_life_state > ls_alive_unconscious)
		return;


	if (m_current_life < m_max_life)
	{
		m_current_life = min(m_current_life + m_life_recovery_unit, m_max_life);
	}
	if ((m_current_life >= m_max_life) &&
		(m_life_state == ls_alive_unconscious))
	{
		PostWorldMessage(we_resurrected, m_go, m_go, "", 0);
	}

	//queue next regen tick if still below max
	if (m_current_life < m_max_life)
	{
		if (m_life_recovery_period > 0 && m_life_recovery_unit > 0)
		{
			int64_t lifePeriodMs = (int64_t)(m_life_recovery_period * 1000.0f);
			PostWorldMessage(we_player_data_changed, m_go, m_go, "life", lifePeriodMs);
		}
	}
}

void GoAspect::RecoverMana()
{
	if (m_life_state > ls_alive_unconscious)
		return;

	if (m_current_mana < m_max_mana)
	{
		m_current_mana = min(m_current_mana + m_mana_recovery_unit, m_max_mana);
	}

	if (m_current_mana < m_max_mana)
	{
		if (m_mana_recovery_period > 0 && m_mana_recovery_unit > 0)
		{
			int64_t manaPeriodMs = (int64_t)(m_mana_recovery_period * 1000.0f);
			PostWorldMessage(we_player_data_changed, m_go, m_go, "mana", manaPeriodMs);
		}
	}
}