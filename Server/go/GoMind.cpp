#include "Go.hpp"
#include "GoMind.h"
#include "../utils/AIQuery.h"
#include <iostream>
#include "../helper/Helper.h"

GoMind :: GoMind (Go * go) : GoComponent (go)
{
	m_melee = 0;
}

GoMind :: GoMind (Go * go, xmlNode * node) : GoComponent (go)
{
	m_melee = 0;
}

GoMind::GoMind(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
	if (tmplComp == nullptr)
		return;

	const string* f;
	if (f = tmplComp->GetField("actor_auto_defends_others")) { if (FromString(*f, m_actor_auto_defends_others) != true) m_actor_auto_defends_others = false; }
	if (f = tmplComp->GetField("actor_auto_heals_others_life")) { if (FromString(*f, m_actor_auto_heals_others_life) != true) m_actor_auto_heals_others_life = false; }
	if (f = tmplComp->GetField("actor_auto_switches_to_karate")) { if (FromString(*f, m_actor_auto_switches_to_karate) != true) m_actor_auto_switches_to_karate = false; }
	if (f = tmplComp->GetField("actor_auto_switches_to_melee")) { if (FromString(*f, m_actor_auto_switches_to_melee) != true) m_actor_auto_switches_to_melee = false; }
	//eWeaponPreference actor_weapon_preference;

	if (f = tmplComp->GetField("actor_life_ratio_high_threshold")) { try { m_actor_life_ratio_high_threshold = std::stof(*f); } catch (...) { m_actor_life_ratio_high_threshold = 0.0f; } }
	if (f = tmplComp->GetField("actor_life_ratio_low_threshold")) { try { m_actor_life_ratio_low_threshold = std::stof(*f); } catch (...) { m_actor_life_ratio_low_threshold = 0.0f; } }
	if (f = tmplComp->GetField("actor_mana_ratio_high_threshold")) { try { m_actor_mana_ratio_high_threshold = std::stof(*f); } catch (...) { m_actor_mana_ratio_high_threshold = 0.0f; } }
	if (f = tmplComp->GetField("actor_mana_ratio_low_threshold")) { try { m_actor_mana_ratio_low_threshold = std::stof(*f); } catch (...) { m_actor_mana_ratio_low_threshold = 0.0f; } }

	if (f = tmplComp->GetField("actor_may_attack")) { if (FromString(*f, m_actor_may_attack) != true) m_actor_may_attack = false; }
	if (f = tmplComp->GetField("actor_may_be_attacked")) { if (FromString(*f, m_actor_may_be_attacked) != true) m_actor_may_be_attacked = false; }

	if (f = tmplComp->GetField("jat_brain")) { try { m_jat_brain = *f; } catch (...) { m_jat_brain = ""; } }
	if (f = tmplComp->GetField("jat_listen")) { try { m_jat_listen = *f; } catch (...) { m_jat_listen = ""; } }
	if (f = tmplComp->GetField("melee_engage_range")) { try { m_melee_engage_range = std::stof(*f); } catch (...) { m_melee_engage_range = 0.0f; } }
	//eMovementOrders movement_orders = mo_limited;

	if (f = tmplComp->GetField("on_enemy_entered_icz_switch_to_melee")) { if (FromString(*f, m_on_enemy_entered_icz_switch_to_melee) != true) m_on_enemy_entered_icz_switch_to_melee = false; }
	if (f = tmplComp->GetField("on_engaged_lost_consciousness_abort_attack")) { if (FromString(*f, m_on_engaged_lost_consciousness_abort_attack) != true) m_on_engaged_lost_consciousness_abort_attack = false; }

	if (f = tmplComp->GetField("ranged_engage_range")) { try { m_ranged_engage_range = std::stof(*f); } catch (...) { m_ranged_engage_range = 0.0f; } }
	if (f = tmplComp->GetField("sensor_scan_period")) { try { m_sensor_scan_period = std::stof(*f); } catch (...) { m_sensor_scan_period = 0.0f; } }
	if (f = tmplComp->GetField("sight_range")) { try { m_sight_range = std::stof(*f); } catch (...) { m_sight_range = 0.0f; } }
}

int64_t GoMind :: TimeElapsedSinceLastMeleeAttack () const
{
	return m_melee;
}

void GoMind :: Stop ()
{
	m_jat = jat_none;
	m_position = m_go->Placement()->Position();
	m_distance = 0.0f;
	m_object = NULL;
	m_slot = es_none;
}

void GoMind :: Move (const SiegePos & position)
{
	m_jat = jat_move;
	m_position = position;
	m_distance = 0.0f;
	m_object = NULL;
	m_slot = es_none;
	
	SendWorldMessage (we_mind_processing_new_job, m_go, m_go, "jat_move");
}

void GoMind :: Get (Go * item)
{
	if (item != NULL)
	{
		if (m_go->Inventory()->Contains(item) != true)
		{
			if (AIQuery::IsInRange (m_go, item, 2.5f))
			{
				m_jat = jat_get;
				m_position = m_go->Placement()->Position();
				m_distance = 0.0f;
				m_object = item;
				m_slot = es_none;
			
				SendWorldMessage (we_mind_processing_new_job, m_go, item, "jat_get");
				
				Stop();
			}
			else
			{
				m_jat = jat_get;
				m_position = item->Placement()->Position();
				m_distance = 2.0f;
				m_object = item;
				m_slot = es_none;
			
				SendWorldMessage (we_mind_processing_new_job, m_go, item, "jat_approach");
			}
			
			return;
		}
	}
	
	Stop();
}

void GoMind :: Drop (Go * item)
{
	if (item != NULL)
	{
		if (m_go->Inventory()->Contains (item))
		{
			if (m_go->Inventory()->IsEquipped (item))
			{
				m_jat = jat_drop;
				m_position = m_go->Placement()->Position();
				m_distance = 0.0f;
				m_object = item;
				m_slot = m_go->Inventory()->GetEquippedSlot (item);
				
				SendWorldMessage (we_mind_processing_new_job, m_go, item, "jat_unequip");
				
				ContinueLastAction();
			}
			else
			{
				m_jat = jat_drop;
				m_position = m_go->Placement()->Position();
				m_distance = 0.0f;
				m_object = item;
				m_slot = es_none;
			
				SendWorldMessage (we_mind_processing_new_job, m_go, item, "jat_drop");
				
				Stop();
			}
			
			return;
		}
	}
	
	Stop();
}

void GoMind :: Equip (eEquipSlot slot, Go * item)
{
	if (item != NULL)
	{
		if (slot != es_none)
		{
			if (slot == es_any)
			{
				slot = item->IntendedSlot();
			    cout << "Equip: auto-mapped item " << item->Goid() << "(location to slot " << slot << endl;
			}


			/*if (item->GetLoc() == il_shield)
			{
				if (m_go->Inventory()->ItemFromLocation(il_active_melee_weapon) != NULL)
				{
					if (m_go->Inventory()->ItemFromLocation(il_active_melee_weapon)->Attack()->IsTwoHanded() == false)
					{
						//maybe use message/event instead of inven function
						m_go->Inventory()->Equip(es_weapon_hand, m_go->Inventory()->ItemFromLocation(il_active_melee_weapon));
					}
					else if (m_go->Inventory()->ItemFromLocation(il_active_melee_weapon)->Attack()->IsTwoHanded())
					{
						// unequip weapon
						//maybe use message/event instead of inven function
						m_go->Inventory()->Unequip(es_weapon_hand);
					}
				}

				if ((m_go->Inventory()->ItemFromLocation(il_active_melee_weapon) != NULL) &&
				  	  (m_go->Inventory()->ItemFromLocation(il_active_melee_weapon)->Attack()->IsTwoHanded() == false))
				{
					m_go->Inventory()->Equip(es_weapon_hand, m_go->Inventory()->ItemFromLocation(il_active_melee_weapon));
				}
			}*/

			if (m_go->Inventory()->Contains (item))
			{
				Go * existing = m_go->Inventory()->GetEquipped (slot);

				if (m_go->Inventory()->IsEquipped (item))
				{
					cout << "eww 1" << endl;
					m_jat = jat_equip;
					m_position = m_go->Placement()->Position();
					m_distance = 0.0f;
					m_object = item;
					m_slot = m_go->Inventory()->GetEquippedSlot (item);

					SendWorldMessage (we_mind_processing_new_job, m_go, item, "jat_unequip");
					
					ContinueLastAction();
				}
				else if (existing != NULL)
				{
					cout << "eww 2" << endl;
					m_jat = jat_equip;
					m_position = m_go->Placement()->Position();
					m_distance = 0.0f;
					m_object = item;
					m_slot = slot;

					SendWorldMessage (we_mind_processing_new_job, m_go, existing, "jat_unequip");
					

					ContinueLastAction();
				}
				else
				{
					if (m_jat == jat_unequip)
					{
						cout << "calling jat_remember_loc" << endl;
						m_object->SetLoc(m_object->IntendedLoc());
					}
					m_jat = jat_equip;
					m_position = m_go->Placement()->Position();
					m_distance = 0.0f;
					m_object = item;
					m_slot = slot;

					SendWorldMessage (we_mind_processing_new_job, m_go, item, "jat_equip");
					
					Stop();
				}
				
				return;
			}
		}
	}
	
	Stop();
}

void GoMind :: Unequip (eEquipSlot slot)
{
	//cout << "GoMind:: Unequip slot " << ToString(slot) << endl;
	Go * item = m_go->Inventory()->GetEquipped (slot);
	if (item != NULL)
	{
		item->SetLoc(il_main);

		m_jat = jat_unequip;
		m_position = m_go->Placement()->Position();
		m_distance = 0.0f;
		m_object = item;
		m_slot = slot;

		SendWorldMessage (we_mind_processing_new_job, m_go, item, "jat_unequip");
	}
	
	//Stop();
}

void GoMind :: AttackMelee (Go * target)
{
	if (target == NULL)
	{
		Stop();
		return;
	}
	if (target->Aspect()->LifeState() > ls_alive_unconscious)
	{
		Stop();
		return;
	}

	int64_t now = CurrentTime();
	{
		if (AIQuery::IsInRange (m_go, target, 2.5f))
		{
			m_jat = jat_attack_object_melee;
			m_position = m_go->Placement()->Position();
			m_distance = 0.0f;
			m_object = target;
			m_slot = es_none;

			SendWorldMessage (we_mind_processing_new_job, m_go, target, "jat_attack_object_melee");
			
			m_melee = now;
		}
		else
		{
			m_jat = jat_attack_object_melee;
			m_position = target->Placement()->Position();
			m_distance = 2.4f;
			m_object = target;
			m_slot = es_none;
		
			SendWorldMessage (we_mind_processing_new_job, m_go, target, "jat_approach");
		}
	}
}

void GoMind :: AttackRanged (Go * target)
{
	if (target != NULL)
	{
		int64_t now = CurrentTime();
		{
			if (AIQuery::IsInRange (m_go, target, 12.5f))
			{
				m_jat = jat_attack_object_ranged;
				m_position = m_go->Placement()->Position();
				m_distance = 0.0f;
				m_object = target;
				m_slot = es_none;
			
				SendWorldMessage (we_mind_processing_new_job, m_go, target, "jat_attack_object_ranged");
				
				m_ranged = now;
			}
			else
			{
				m_jat = jat_attack_object_ranged;
				m_position = target->Placement()->Position();
				m_distance = 12.4f;
				m_object = target;
				m_slot = es_none;
			
				SendWorldMessage (we_mind_processing_new_job, m_go, target, "jat_approach");
			}
		}
		
		return;
	}
	
	Stop();
}

void GoMind :: ContinueLastAction ()
{
	switch (m_jat)
	{
		case jat_get:
		{
			if (m_object != NULL)
			{
				Get (m_object);
			}
		}
		break;
		
		case jat_drop:
		{
			if (m_object != NULL)
			{
				Drop (m_object);
			}
		}
		break;
		
		case jat_equip:
		{
			if (m_object != NULL && m_slot != es_none)
			{
				Equip (m_slot, m_object);
			}
		}
		break;
		
		case jat_attack_object_melee:
		{
			if (m_object != NULL)
			{
				AttackMelee (m_object);
			}
		}
		break;
		
		case jat_attack_object_ranged:
		{
			if (m_object != NULL)
			{
				AttackRanged (m_object);
			}
		}
		break;
		
		default: break;
	} /* switch (m_jat) */
}

eJobAbstractType GoMind :: ActionJat () const
{
	return m_jat;
}

SiegePos GoMind :: ActionPosition () const
{
	return m_position;
}

float GoMind :: ActionDistance () const
{
	return m_distance;
}

Go * GoMind :: ActionObject () const
{
	return m_object;
}

eEquipSlot GoMind :: ActionSlot () const
{
	return m_slot;
}

eInventoryLocation GoMind :: ActionLocation () const
{
	return m_location;
}
