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

#include "Engine.hpp"
#include "events/SendWorldMessageEvent.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>

Engine engine;

Engine :: Engine ()
{
	m_Running = true;
}

Engine :: ~Engine ()
{
	while (m_EventRegistry.size())
	{
		Event * event = m_EventRegistry.top();
		delete event;
		m_EventRegistry.pop();
	}
}

void Engine::RegisterPlayer(Go* playerGo) {
    if (playerGo) m_players.insert(playerGo);
    playerGo->CalculateStatus();
}
void Engine::UnregisterPlayer(Go* playerGo) {
    m_players.erase(playerGo);
}
bool Engine::IsPlayer(Go* obj) const {
    return m_players.find(obj) != m_players.end();
}

void Engine::RegisterItem(Go* itemGo) {
    if (itemGo) m_items.insert(itemGo);
}
void Engine::UnregisterItem(Go* itemGo) {
	m_items.erase(itemGo);
}

void Engine::RegisterPlayerCharacter(Go* pGo) {
    if (pGo) m_playerChars.insert(pGo);
}
void Engine::UnregisterPlayerCharacter(Go* pGo) {
	m_playerChars.erase(pGo);
}


bool Engine :: IsRunning ()
{
	return m_Running;
}

void Engine :: Loop ()
{
	int64_t current = CurrentTime();
	
	while (m_EventRegistry.size() > 0 && m_EventRegistry.top()->ExecutionTime() <= current)
	{
		Event * event = m_EventRegistry.top();
		m_EventRegistry.pop();
		
		try
		{
			event->Execute();
		}
		catch (...)
		{
		}
		
		delete event;
	}

	TryRegenerateAllGos(current);

	if ((CurrentTime() % 60000) == 0) {
		//logger.Write("[ENGINE] ####### [START] 60 Second Timer #######", true);
		for (GopSet::iterator iterator = m_players.begin(); iterator != m_players.end(); iterator++)
		{
			(*iterator)->SaveToXml("actors");
			//logger.Write("[ENGINE] Saving Character ", true);
		}
		logger.Write("[ENGINE] Players saved", true);



		for (GopSet::iterator iterator = m_items.begin(); iterator != m_items.end(); iterator++)
		{
			(*iterator)->SaveToXml("items");
			//logger.Write("[ENGINE] Saving Item ", true);
		}
		logger.Write("[ENGINE] Items saved", true);
		//logger.Write("[ENGINE] ####### [END] 60 Second Timer #######", true);
	}
}

void Engine :: RegisterEvent (Event * event)
{
	m_EventRegistry.push (event);
}

void Engine :: HandleWorldMessage (const WorldMessage & message)
{
	eWorldEvent event = message.WorldEvent();
	Go * from = message.SendFrom();
	Go * to = message.SendTo();
	
	switch (event)
	{
		case we_entered_world:
		{
			if (from->HasPlacement())
			{
				if (message.Data().empty() != true)
				{
					AddGoToRegion (from, message.Data());
				}
			}
		}
		break;
		
		case we_left_world:
		{
			if (from->HasPlacement())
			{
				if (from->Placement()->GetRegion().empty() != true)
				{
					if (engine.IsPlayer(from)) {
						engine.UnregisterPlayer(from);
					}
					RemoveGoFromRegion (from);
				}
			}
		}
		break;
		
		case we_entered_frustum:
		{
			//cout << "we_entered_frustum from: " << from->Goid() << " to: " << to->Goid() << endl;
			if (from->HasPlacement() && to->HasPlacement())
			{
				from->Send (WorldMessage (we_entered_frustum, to, from, ""));
				if (from->Goid() != to->Goid())
				{
					to->Send (WorldMessage (we_entered_frustum, from, to, ""));
				}
			}
		}
		break;
		
		case we_left_frustum:
		{
			//cout << "we_left_frustum from: " << from->Goid() << " to: " << to->Goid() << endl;
			if (from->HasPlacement() && to->HasPlacement())
			{
				from->Send (WorldMessage (we_left_frustum, to, from, ""));
				if (from->Goid() != to->Goid())
				{
					to->Send (WorldMessage (we_left_frustum, from, to, ""));
				}
			}
		}
		break;
		
		case we_picked_up:
		{
			if (from->HasInventory() && to->IsItem())
			{
				string region = to->Placement()->GetRegion();
				if (region.empty() != true)
				{
					RemoveGoFromRegion (to);

				}
				
				to->Send (message); // scripting event
				from->Send (message);
			}
		}
		break;
		
		case we_dropped:
		{
			if (from->HasInventory() && to->IsItem())
			{
				string region = from->Placement()->GetRegion();
				if (region.empty() != true)
				{
					to->Placement()->SetRegion(region);
					to->Placement()->SetPosition (from->Placement()->Position());
					AddGoToRegion (to, region);
				}
				
				to->Send (message); // scripting event
				from->Send (message);
			}
		}
		break;
		
		case we_equipped:
		{
			if (from->IsActor() && to->IsItem())
			{
				to->Send (message); // scripting event
				from->Send (message);
			}
		}
		break;
		
		case we_unequipped:
		{
			if (from->IsActor() && to->IsItem())
			{
				to->Send (message); // scripting event
				from->Send (message);
			}
		}
		break;
		
		case we_req_talk:
		{
			MessageKnown (from, message);
		}
		break;
		
		case we_resurrected:
		{
			from->Aspect()->SetLifeState(ls_alive_conscious);
			from->Aspect()->SetCurrentLife(from->Aspect()->MaxLife());
			from->Aspect()->SetLastDied(0);
			UpdateGo(from);
			//to->Send (message); // scripting event
			//from->Send (message);
			// scripting
		}
		break;

		case we_engaged_hit_lived:
		{
			MessageKnown (from, message);
			// scripting
		}
		break;
		
		case we_engaged_missed:
		{
			MessageKnown (from, message);
			// scripting
		}
		break;
		
		case we_mind_processing_new_job:
		{
			string job = message.Data();
			
			if (job == "")
			{
				from->Mind()->ContinueLastAction();
			}
			
			else if (job == "jat_move")
			{
				// if canLeaveCurrentRegion (from) && canEnterNextRegion (from)
				MessageKnown (from, message);
			}
			
			else if (job == "jat_approach")
			{
				MessageKnown (from, message);
			}
			
			else if (job == "jat_get")
			{
				if (from->IsActor() && to->IsItem())
				{
					if (from->Inventory()->Add (to))
					{
						HandleWorldMessage (WorldMessage (we_picked_up, from, to, ""));
					}
					else
					{
						from->Mind()->Stop();
					}
				}
			}
			
			else if (job == "jat_drop")
			{
				if (from->IsActor() && to->IsItem())
				{
					if (from->Inventory()->Remove (to))
					{
						HandleWorldMessage (WorldMessage (we_dropped, from, to, ""));
					}
					else
					{
						from->Mind()->Stop();
					}
				}
			}
			
			else if (job == "jat_equip")
			{
				if (from->IsActor() && to->IsItem())
				{
					eEquipSlot slot = from->Mind()->ActionSlot();
					if (from->Inventory()->Equip (slot, to))
					{
						MessageKnown (from, message);
						HandleWorldMessage (WorldMessage (we_equipped, from, to, ""));
					}
					else
					{
						from->Mind()->Stop();
					}
				}
			}
			
			else if (job == "jat_unequip")
			{
				if (from->IsActor() && to->IsItem())
				{
					eEquipSlot slot = from->Mind()->ActionSlot();
					cout << "engine says unequip " << to->Goid() << " from " << slot << endl;
					if (from->Inventory()->Unequip (slot))
					{
						cout << "successfully unequipped" << endl;
						MessageKnown (from, message);
						HandleWorldMessage (WorldMessage (we_unequipped, from, to, ""));

						//to->SetLoc(il_main);
					}
					else
					{
						cout << "successfully unequipped stopping now" << endl;
						from->Mind()->Stop();
					}
				}
			}
			
			else if (job == "jat_attack_object_melee")
			{
				if (!from->IsActor() || !to->IsActor()) // fix me!
					return;

				GoAttack * attacker = from->Attack();
				//GoAspect * fromAspect = from->Aspect();
				GoAspect * toAspect = to->Aspect();
				GoInventory * fromInv = from->Inventory();

				const int hitType = attacker->CalcHitType(to, "melee");
				const uint64_t attTime = attacker->CalcHitTime();
				cout << "attTime: " << attTime << endl;
				if (hitType == 0)
				{
					HandleWorldMessage (WorldMessage (we_engaged_missed, from, to, ""));
					PostWorldMessage (we_mind_processing_new_job, from, to, "", attTime);
					return;
				}

				float hp = toAspect->CurrentLife();
				float dmg = attacker->CalcDamage(to, "melee", hitType, nullptr);
				hp -= dmg;

				if (hp <= 0)
				{
					if (toAspect->LifeState() < ls_alive_unconscious)
						toAspect->SetLifeState(ls_alive_unconscious);

					if (hp <= -(toAspect->MaxLife() * 2.0f / 3.0f))
						toAspect->SetLifeState(ls_dead_normal);
				}
				toAspect->SetCurrentLife(hp);

				// Add exp to attacker used skill
				if (from->Actor()->CanLevelUp()/*IsPlayer(to) == false*/)
				{
					{
						float targetExp = toAspect->ExperienceValue();
						if (targetExp == 0)
							targetExp = 100.0f;

						float addExp = targetExp / toAspect->MaxLife() * dmg;
						string skill = "melee"; // default

						switch (fromInv->GetSelectedSlot())
						{
							case 1: skill = "melee"; break;
							case 2: skill = "ranged"; break;
							case 3:
							case 4:
							{
								Go * item = fromInv->ItemFromLocation((eInventoryLocation)(fromInv->GetSelectedSlot() + 1));
								if (item)
									skill = item->Magic()->SkillClass();
								break;
							}
							default: break;
						}
						float maxGain = from->Actor()->GetMaxExpGainForLevel(from->Actor()->GetSkillLevel(skill));
						if (addExp > maxGain)
							addExp = maxGain;

						from->Actor()->AddSkillExp(skill, addExp);
						UpdateGoExp(from, addExp);
					}
				}

				UpdateGo(to);
				HandleWorldMessage (WorldMessage (we_engaged_hit_lived, from, to, ""));
				if (hp > 0)
					PostWorldMessage (we_mind_processing_new_job, from, to, "", attTime);

			}
			
			else if (job == "jat_attack_object_ranged")
			{
				if (!from->IsActor() || !to->IsActor()) // fix me!
					return;

				GoAttack * attacker = from->Attack();
				//GoAspect * fromAspect = from->Aspect();
				GoAspect * toAspect = to->Aspect();
				GoInventory * fromInv = from->Inventory();

				const int hitType = attacker->CalcHitType(to, "ranged");
				const uint64_t attTime = attacker->CalcHitTime();

				if (hitType == 0)
				{
					HandleWorldMessage (WorldMessage (we_engaged_missed, from, to, ""));
					PostWorldMessage (we_mind_processing_new_job, from, to, "", attTime);
					return;
				}
				float hp = toAspect->CurrentLife();
				float dmg = attacker->CalcDamage(to, "ranged", hitType, nullptr);
				hp -= dmg;

				if (hp <= 0)
				{
					if (toAspect->LifeState() < ls_alive_unconscious)
						toAspect->SetLifeState(ls_alive_unconscious);

					if (hp <= -(toAspect->MaxLife() * 2.0f / 3.0f))
						toAspect->SetLifeState(ls_dead_normal);
				}

				to->Aspect()->SetCurrentLife(hp);

				// Add exp to attacker used skill
				if (from->Actor()->CanLevelUp()/*IsPlayer(to) == false*/)
				{
					if (from->Inventory()->GetEquipped(es_shield_hand) != nullptr)
					{
						float targetExp = toAspect->ExperienceValue();
						if (targetExp == 0)
							targetExp = 100.0f;

						float addExp = targetExp / toAspect->MaxLife() * dmg;
						std::string skill = "melee"; // default

						switch (fromInv->GetSelectedSlot())
						{
							case 1: skill = "melee"; break;
							case 2: skill = "ranged"; break;
							case 3:
							case 4:
							{
								Go * item = fromInv->ItemFromLocation((eInventoryLocation)(fromInv->GetSelectedSlot() + 1));
								if (item)
									skill = item->Magic()->SkillClass();
								break;
							}
							default: break;
						}
						float maxGain = from->Actor()->GetMaxExpGainForLevel(from->Actor()->GetSkillLevel(skill));
						if (addExp > maxGain)
							addExp = maxGain;

						from->Actor()->AddSkillExp(skill, addExp);
						UpdateGoExp(from, addExp);
					}
				}
				UpdateGo(to);
				HandleWorldMessage (WorldMessage (we_engaged_hit_lived, from, to, ""));
				if (hp > 0)
					PostWorldMessage (we_mind_processing_new_job, from, to, "", attTime);

			}
		}
		break;
		
		case we_frustum_active_state_changed:
		{
			CalculateFrustums ();
			
			PostWorldMessage (we_frustum_active_state_changed, NULL, NULL, "", 1000);
		}
		break;
		
		default: break;
	} /* switch (event) */
}

void Engine :: MessageKnown (Go * go, const WorldMessage & message)
{
	if (go == NULL)
		return;

	const GopSet & frustum = go->Frustum();
	for (GopSet::const_iterator iterator = frustum.begin(); iterator != frustum.end(); iterator++)
	{
		(*iterator)->Send (message);
	}
}

void Engine :: MessageAllPlayers (const WorldMessage & message)
{
	for (GopSet::iterator iterator = m_players.begin(); iterator != m_players.end(); iterator++)
	{
		(*iterator)->Send (message); 
	}
}

void Engine :: UpdateGo(Go* go)
{
	if (go == nullptr)
		return;

	const GopSet & frustum = go->Frustum();
	for (GopSet::const_iterator iterator = frustum.begin(); iterator != frustum.end(); iterator++)
	{
		(*iterator)->Send (WorldMessage(we_go_status_updated, go, (*iterator), ""));
	}
}

void Engine :: UpdateGoHpMp(Go* go, float hp, float mp)
{
	if (go == nullptr)
		return;

	const GopSet & frustum = go->Frustum();
	for (GopSet::const_iterator iterator = frustum.begin(); iterator != frustum.end(); iterator++)
	{
		(*iterator)->Send (WorldMessage(we_go_life_updated, go, (*iterator), ""));
	}
}

void Engine :: UpdateGoExp(Go* go, float value)
{
	if (go == nullptr)
		return;

	const GopSet & frustum = go->Frustum();
	for (GopSet::const_iterator iterator = frustum.begin(); iterator != frustum.end(); iterator++)
	{
		(*iterator)->Send (WorldMessage(we_add_exp, go, (*iterator), std::to_string(value)));
	}
}

void Engine :: UpdateGoLvlup(Go* go, const string & data)
{
	if (go == nullptr)
		return;

	const GopSet & frustum = go->Frustum();
	for (GopSet::const_iterator iterator = frustum.begin(); iterator != frustum.end(); iterator++)
	{
		(*iterator)->Send (WorldMessage(we_leveled_up, go, (*iterator), data));
	}
}


void Engine :: AddGoToRegion (Go * go, const string & data)
{
	//if (!godb.FindGoById(go->Goid()))
	//		return;

	Region * region;
	if (go == NULL)
		return;

	if (data.empty() == true)
		return;

	try
	{
		region = world.GetRegion (data);
		go->Placement()->SetRegion (data);
	}
	catch (exception & e)
	{
		// log a useful error
		return;
	}

	GopSet & available = region->Objects();
	available.insert (go);

	for (GopSet::iterator iterator = available.begin(); iterator != available.end(); iterator++)
	{
		Go * object = *iterator;

		if (query.IsInRange (go, object, 45.0))
		{
			HandleWorldMessage (WorldMessage (we_entered_frustum, go, object, ""));
		}
	}

	if (go->HasComponent ("player"))
	{
		/*
		if (region->Players().size() == 1)
		{
			world.Regions().insert (region);
		}
		*/
	}


}

void Engine :: RemoveGoFromRegion (Go * go)
{
	Region * region;

	if (!godb.FindGoById(go->Goid()))
		return;

	if (go != NULL)
	{
		try
		{
			region = world.GetRegion (go->Placement()->GetRegion());
			if (go->IsItem())
				go->Placement()->SetRegion ("");
			//else
			// transition to new zone
		}
		catch (exception & e)
		{
			// log useful error
			return;
		}
		
		GopSet & available = region->Objects();
		available.erase (go);
		
		const GopSet & frustum = go->Frustum();
		for (GopSet::const_iterator iterator = frustum.begin(); iterator != frustum.end(); iterator++)
		{
			HandleWorldMessage (WorldMessage (we_left_frustum, go, *iterator, ""));
		}
		
		if (go->HasComponent ("player"))
		{
			/*
			if (region->Players().size() == 0)
			{
				world.Regions().erase (region);
			}
			*/
		}
	}
}

void Engine::CalculateFrustums()
{
    set<Region*>& regions = world.Regions();

    for (set<Region*>::iterator region_iter = regions.begin(); region_iter != regions.end(); ++region_iter)
    {
        Region* region = *region_iter;
        GopSet& objects = region->Objects();

        for (GopSet::iterator i = objects.begin(); i != objects.end(); ++i)
        {
            Go* object = *i;

            for (GopSet::iterator j = objects.begin(); j != objects.end(); ++j)
            {
                Go* go = *j;

                if (go == object)
                    continue; // Skip comparing object with itself

                //std::cout << "Calculating Frustum between GO " << object->Goid()
                 //         << " and GO " << go->Goid() << std::endl;

                // Only perform update if object has moved
                if (go->Placement()->IsDirty())
                {
                    //std::cout << go->Goid() << " is dirty, checking against " << object->Goid() << std::endl;

                    bool is_in_frustum = object->Frustum().find(go) != object->Frustum().end();
                    bool should_be_in_frustum = query.IsInRange(go, object, 45.0);

                    if (is_in_frustum && !should_be_in_frustum)
                    {
                        HandleWorldMessage(WorldMessage(we_left_frustum, object, go, ""));
                    }
                    else if (!is_in_frustum && should_be_in_frustum)
                    {
                        HandleWorldMessage(WorldMessage(we_entered_frustum, object, go, ""));
                    }
                    go->Placement()->MarkAsClean();
                }
            }
        }
    }
}

void Engine::TryRegenerateAllGos(int64_t current)
{
	// Regeneration logic
	auto regenerate = [&](Go* go)
	{
		if (!go || go->IsItem()) return;

		GoAspect* aspect = go->Aspect();
		if (!aspect) return;

		if (aspect->LifeState() > ls_alive_unconscious)
			return;

		bool updated = false;

		// Life regen
		int64_t lifePeriodMs = (int64_t)(aspect->LifeRecoveryPeriod() * 1000.0f);
		if (lifePeriodMs > 0.0f && current >= aspect->LastLifeReg() + lifePeriodMs) {
			float currentLife = aspect->CurrentLife();
			float maxLife = aspect->MaxLife();
			float unit = aspect->LifeRecoveryUnit();

			if (currentLife < maxLife) {
				currentLife = std::min(currentLife + unit, maxLife);
				aspect->SetCurrentLife(currentLife);
				updated = true;
				//cout << "Regging " << unit << " hp for Go " << go->Goid() << endl;
			}
			else if (currentLife > maxLife)
			{
				aspect->SetCurrentLife(maxLife);
				updated = true;
			}

			if ((currentLife >= maxLife) &&
				(aspect->LifeState() == ls_alive_unconscious)) {
				PostWorldMessage(we_resurrected, go, go, "", 0);
			}

			aspect->SetLastLifeReg(current);
		}

		// Mana regen
		int64_t manaPeriodMs = (int64_t)(aspect->ManaRecoveryPeriod() * 1000.0f);
		if (manaPeriodMs > 0.0f && current >= aspect->LastManaReg() + manaPeriodMs) {
			float currentMana = aspect->CurrentMana();
			float maxMana = aspect->MaxMana();
			float unit = aspect->ManaRecoveryUnit();

			if (currentMana < maxMana) {
				currentMana = std::min(currentMana + unit, maxMana);
				aspect->SetCurrentMana(currentMana);
				updated = true;
				//cout << "Regging " << unit << " mp for Go " << go->Goid() << endl;
			}
			else if (currentMana > maxMana)
			{
				aspect->SetCurrentMana(maxMana);
				updated = true;
			}

			aspect->SetLastManaReg(current);
		}

		if (updated) {
			UpdateGo(go);
		}
	};

	// Iterate players
	GopSet::iterator it_player;
	for (it_player = m_players.begin(); it_player != m_players.end(); ++it_player)
	{
		regenerate(*it_player);
	}

	// Iterate mobs — future-proofed; define m_mobs in Engine later
	/*GopSet::iterator it_mob;
	for (it_mob = m_mobs.begin(); it_mob != m_mobs.end(); ++it_mob)
	{
		regenerate(*it_mob);
	}*/
}
