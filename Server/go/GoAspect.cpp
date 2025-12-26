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
}

GoAspect::GoAspect(Go* go, const GoAspect& other) : GoComponent(go)
{
	// Copy all primitive members
	m_bounding_sphere_radius = other.m_bounding_sphere_radius;
	m_current_life = other.m_current_life;
	m_current_mana = other.m_current_mana;
	m_invincible = other.m_invincible;
	m_visible = other.m_visible;
	m_life_recovery_period = other.m_life_recovery_period;
	m_life_recovery_unit = other.m_life_recovery_unit;
	m_life_state = other.m_life_state;
	m_last_died = other.m_last_died;
	m_mana_recovery_period = other.m_mana_recovery_period;
	m_mana_recovery_unit = other.m_mana_recovery_unit;
	m_max_life = other.m_max_life;
	m_max_mana = other.m_max_mana;
	m_model = other.m_model; // string copies automatically
	m_render_scale = other.m_render_scale;
	m_experience_value = other.m_experience_value;

	// Copy the texture array
	for (int i = 0; i < 2; ++i)
		m_textures[i] = other.m_textures[i];
}

GoAspect::GoAspect(Go* go, xmlNode* node) : GoComponent(go)
{
	if (node != NULL)
	{
		xmlNode* current = NULL;
		for (current = node->children; current != NULL; current = current->next)
		{
			if (current->type != XML_ELEMENT_NODE) continue;

			if (xmlStrEqual(current->name, (const xmlChar*)"bounding_sphere_radius") != 0)
			{
				m_bounding_sphere_radius = xml::ReadAttribute<float>(current, "value", 0.0);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"current_life") != 0)
			{
				m_current_life = xml::ReadAttribute<float>(current, "value", 0.0);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"current_mana") != 0)
			{
				m_current_mana = xml::ReadAttribute<float>(current, "value", 0.0);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"flesh") != 0)
			{
				m_textures[0] = xml::XReadString(current, "value", "");
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"cloth") != 0)
			{
				m_textures[1] = xml::XReadString(current, "value", "");
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"is_invincible") != 0)
			{
				m_invincible = xml::ReadAttribute<bool>(current, "value", false);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"is_visible") != 0)
			{
				m_visible = xml::ReadAttribute<float>(current, "value", true);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"life_recovery_period") != 0)
			{
				m_life_recovery_period = xml::ReadAttribute<int16_t>(current, "value", 4);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"life_recovery_unit") != 0)
			{
				m_life_recovery_unit = xml::ReadAttribute<float>(current, "value", 1.0);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"life_state") != 0)
			{
				string life_state = xml::XReadString(current, "value", "");
				if (FromString(life_state, m_life_state) != true)
				{
					m_life_state = ls_alive_conscious;
				}
				else
				{
					m_life_state = ToState(life_state);
				}
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"last_died") != 0)
			{
				m_last_died = xml::ReadAttribute<int64_t>(current, "value", 0);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"mana_recovery_period") != 0)
			{
				m_mana_recovery_period = xml::ReadAttribute<int16_t>(current, "value", 3);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"mana_recovery_unit") != 0)
			{
				m_mana_recovery_unit = xml::ReadAttribute<float>(current, "value", 1.0);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"max_life") != 0)
			{
				m_max_life = xml::ReadAttribute<float>(current, "value", 0.0);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"max_mana") != 0)
			{
				m_max_mana = xml::ReadAttribute<float>(current, "value", 0.0);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"model") != 0)
			{
				m_model = xml::XReadString(current, "value", "");
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"render_scale") != 0)
			{
				m_render_scale = xml::ReadAttribute<float>(current, "value", 1.0);
			}
			else if (xmlStrEqual(current->name, (const xmlChar*)"experience_value") != 0)
			{
				m_experience_value = xml::ReadAttribute<float>(current, "value", 1.0);
			}
		}
	}
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

void GoAspect::InheritFrom(const GoAspect& other)
{
	// Numeric members: inherit if still default
	if (m_render_scale == 1.0f) m_render_scale = other.m_render_scale;
	if (m_life_recovery_period == 4) m_life_recovery_period = other.m_life_recovery_period;
	if (m_mana_recovery_period == 4) m_mana_recovery_period = other.m_mana_recovery_period;

	if (m_bounding_sphere_radius == 0.0f) m_bounding_sphere_radius = other.m_bounding_sphere_radius;
	if (m_max_life == 0.0f) m_max_life = other.m_max_life;
	if (m_max_mana == 0.0f) m_max_mana = other.m_max_mana;
	if (m_current_life == m_max_life) m_current_life = other.m_current_life;
	if (m_current_mana == m_max_mana) m_current_mana = other.m_current_mana;

	// Textures
	if (m_textures[0].empty()) m_textures[0] = other.m_textures[0];
	if (m_textures[1].empty()) m_textures[1] = other.m_textures[1];

	// Bool members
	if (!m_invincible) m_invincible = other.m_invincible;
	if (m_visible == true) m_visible = other.m_visible;

	// Recovery units
	if (m_life_recovery_unit == 0.0f) m_life_recovery_unit = other.m_life_recovery_unit;
	if (m_mana_recovery_unit == 0.0f) m_mana_recovery_unit = other.m_mana_recovery_unit;

	// Life state
	if (m_life_state == ls_alive_conscious) m_life_state = other.m_life_state;

	// Last died
	if (m_last_died == 0) m_last_died = other.m_last_died;

	// Model
	if (m_model.empty()) m_model = other.m_model;

	// Experience
	if (m_experience_value == 1.0f) m_experience_value = other.m_experience_value;
}


void GoAspect::Save(xmlNode* aspectNode) const
{
	xml::SetOrUpdateChildValue(aspectNode, "bounding_sphere_radius", m_bounding_sphere_radius);
	xml::SetOrUpdateChildValue(aspectNode, "current_life", m_current_life);
	xml::SetOrUpdateChildValue(aspectNode, "current_mana", m_current_mana);
	xml::SetOrUpdateChildValue(aspectNode, "flesh", m_textures[0]);
	xml::SetOrUpdateChildValue(aspectNode, "cloth", m_textures[1]);
	xml::SetOrUpdateChildValue(aspectNode, "life_state", ToString(m_life_state));
	xml::SetOrUpdateChildValue(aspectNode, "last_died", m_last_died);
	xml::SetOrUpdateChildValue(aspectNode, "max_life", m_max_life);
	xml::SetOrUpdateChildValue(aspectNode, "max_mana", m_max_mana);
	xml::SetOrUpdateChildValue(aspectNode, "model", m_model);
	xml::SetOrUpdateChildValue(aspectNode, "render_scale", m_render_scale);
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