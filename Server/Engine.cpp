#include "Engine.hpp"
#include "events/SendWorldMessageEvent.hpp"

#include "utils/AIQuery.h"

Engine g_engine;

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

	if ((CurrentTime() % 60000) == 0) {
		//Log::Write("[ENGINE] ####### [START] 60 Second Timer #######", true);
		for (GopSet::iterator iterator = m_players.begin(); iterator != m_players.end(); iterator++)
		{
			(*iterator)->SaveToXml("actors");
			//Log::Write("[ENGINE] Saving Character ", true);
		}
		Log::Write(Log::Level::INFO, "[ENGINE] Players saved", true);



		for (GopSet::iterator iterator = m_items.begin(); iterator != m_items.end(); iterator++)
		{
			(*iterator)->SaveToXml("items");
			//Log::Write("[ENGINE] Saving Item ", true);
		}
		Log::Write(Log::Level::INFO, "[ENGINE] Items saved", true);
		//Log::Write("[ENGINE] ####### [END] 60 Second Timer #######", true);
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
					if (g_engine.IsPlayer(from)) {
						g_engine.UnregisterPlayer(from);
					}
					RemoveGoFromRegion (from);
				}
			}
		}
		break;

		case we_frustum_active_state_changed:
		{
			CalculateFrustums();

			PostWorldMessage(we_frustum_active_state_changed, NULL, NULL, "", 1000);
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
		
		case we_player_data_changed:
		{
			string dataType = message.Data();

			if (dataType == "life")
			{
				if (from->HasAspect())
				{
					from->Aspect()->RecoverLife();
					UpdateGo(from);
				}
			}
			else if (dataType == "mana")
			{
				if (from->HasAspect())
				{
					from->Aspect()->RecoverMana();
					UpdateGo(from);
				}
			}

			/*if (from->HasActor())
			{
				from->Aspect()->RecoverLife
				UpdateGo(from);
			}*/
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

			//MessageKnown(from, WorldMessage(we_go_status_updated, from, to, ""));
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
			
			else if (job == "jat_stop")
			{
				// if canLeaveCurrentRegion (from) && canEnterNextRegion (from)
				MessageKnown (from, message);
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
				cout << "jat_attack_object_melee" << endl;
				if (!from->IsActor() || !to->IsActor())
					return;

				if (to->Aspect()->CurrentLife() <= -(to->Aspect()->MaxLife() * 2.0f / 3.0f))
				{
					PostWorldMessage (we_mind_processing_new_job, from, to, "jat_stop", 0);
					return;
				}

				GoAttack * attacker = from->Attack();
				const int hitType = attacker->CalcHitType(to, "melee");
				const uint64_t attTime = attacker->CalcHitTime();

				if (hitType == 0)
				{
					HandleWorldMessage (WorldMessage (we_engaged_missed, from, to, ""));
					PostWorldMessage (we_mind_processing_new_job, from, to, "", attTime);
					return;
				}

				// Show animation hit event now
				HandleWorldMessage (WorldMessage (we_engaged_hit_lived, from, to, ""));

				// Schedule actual damage application at 50% of the attack time
				std::string data = std::to_string(hitType);
				PostWorldMessage (we_weapon_swung, from, to, data, attTime / 2);

				// Schedule next attack decision at full attack time
				PostWorldMessage (we_mind_processing_new_job, from, to, "", attTime);

			}
			
			else if (job == "jat_attack_object_ranged")
			{
				cout << "jat_attack_object_ranged" << endl;

				if (!from->IsActor() || !to->IsActor())
					return;

				Go * weapon = from->Inventory()->ItemFromLocation(il_active_ranged_weapon);//from->Inventory()->GetEquipped(es_shield_hand);

				if (!weapon || !weapon->HasAttack())
					return;

				float distance = (float)from->GetDistanceTo(to);
				float attRange = weapon->Attack()->AttackRange() + from->Attack()->AttackRange();
				if (distance > attRange)
					return;

				if (to->Aspect()->CurrentLife() <= -(to->Aspect()->MaxLife() * 2.0f / 3.0f))
				{
					PostWorldMessage (we_mind_processing_new_job, from, to, "jat_stop", 0);
					return;
				}

				GoAttack * attacker = from->Attack();
				const int hitType = 1;//attacker->CalcHitType(to, "ranged");
				const uint64_t attTime = attacker->CalcHitTime();
				uint64_t fireTime = 300;
				cout << "reload delay from wep: " << weapon->Attack()->ReloadDelay() << endl;
				fireTime = (weapon->Attack()->ReloadDelay() * 1750);//(weapon->Attack()->ReloadDelay() * 1000.0f) + ((distance * 1000) / 14);
				cout << "calculated fire time: " << fireTime << " distance: " << (double)distance << " distance delay: " << ((distance * 1000) / 14) << " attTime: " << attTime << endl;


				if (hitType == 0)
				{
					HandleWorldMessage (WorldMessage (we_engaged_missed, from, to, ""));
					PostWorldMessage (we_mind_processing_new_job, from, to, "", attTime);
					return;
				}

				// Show animation hit event now
				HandleWorldMessage (WorldMessage (we_engaged_hit_lived, from, to, ""));

				// Schedule actual damage application at 50% of the attack time
				std::string data = std::to_string(hitType);
				PostWorldMessage (we_weapon_launched, from, to, data, fireTime);

				// Schedule next attack decision at full attack time
				PostWorldMessage (we_mind_processing_new_job, from, to, "", attTime);
			}
		}
		break;
		
		case we_weapon_swung:
		{
			if (from->Mind()->ActionJat() != jat_attack_object_melee)
				return;

			GoAttack * attacker = from->Attack();
			GoAspect * toAspect = to->Aspect();
			GoInventory * fromInv = from->Inventory();

			int hitType = std::stoi(message.Data());  // passed from earlier
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

			if (toAspect->CurrentLife() >= toAspect->MaxLife())
			{
				int64_t lifePeriodMs = (int64_t)(toAspect->LifeRecoveryPeriod() * 1000.0f);
				PostWorldMessage(we_player_data_changed, to, to, "life", lifePeriodMs);
			}

			toAspect->SetCurrentLife(hp);
			UpdateGo(to);

			// Add exp
			if (!from->Actor()->CanLevelUp())
				return;

			float targetExp = toAspect->ExperienceValue();
			if (targetExp == 0) targetExp = 100.0f;
			float addExp = targetExp / toAspect->MaxLife() * dmg;

			std::string skill = "melee";
			switch (fromInv->GetSelectedSlot())
			{
				case 1: skill = "melee"; break;
				case 2: skill = "ranged"; break;
				case 3:
				case 4:
				{
					Go * item = fromInv->ItemFromLocation((eInventoryLocation)(fromInv->GetSelectedSlot() + 1));
					if (item) skill = item->Magic()->SkillClass();
					break;
				}
			}
			float maxGain = from->Actor()->GetMaxExpGainForLevel(from->Actor()->GetSkillLevel(skill));
			if (addExp > maxGain) addExp = maxGain;

			from->Actor()->AddSkillExp(skill, addExp);
			UpdateGoExp(from, addExp);
		}
		break;

		case we_weapon_launched:
		{
			cout << "bow fired at: " << CurrentTime() << endl;
			if (from->Mind()->ActionJat() != jat_attack_object_ranged)
			{
				cout << "bow attack canceled" << endl;
				return;
			}
			// now after we already checked we can queue a message with new delay for distance
			double distance = from->GetDistanceTo(to);
			int velocity = 25;
			if (from->Inventory()->GetEquipped(es_shield_hand)->Attack()->AttackClass() == ac_minigun)
				velocity = 75;

			uint64_t airTime = (uint64_t)((distance * 1000) / velocity);
			cout << "calculated air time: " << airTime << " distance: " << (double)distance << " distance delay: " << ((distance * 1000) / 14) << endl;

			PostWorldMessage (we_projectile_hit, from, to, message.Data(), airTime);
		}
		break;

		case we_projectile_hit:
		{
			cout << "bow hit at: " << CurrentTime() << endl;
			/*double distance = from->GetDistanceTo(to);
			uint64_t airTime = (uint64_t)((distance * 1000) / 14);
			uint64_t momentOfFire = CurrentTime() - airTime;
			uint64_t fireTime = (from->Inventory()->GetEquipped(es_shield_hand)->Attack()->ReloadDelay() * 1000.0f);
			uint64_t attStartTime = momentOfFire - fireTime - airTime;
			cout << "moment of fire: " << momentOfFire << " fireTime: " << fireTime << " attStartTime: " << attStartTime << " diff: " << (momentOfFire - (attStartTime + 520)) << endl;
			if (momentOfFire > (attStartTime + 520))
			{
				cout << "bow attack canceled in post" << endl;
				return;
			}*/

			GoAttack * attacker = from->Attack();
			GoAspect * toAspect = to->Aspect();
			GoInventory * fromInv = from->Inventory();

			int hitType = std::stoi(message.Data());  // passed from earlier
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
			if (toAspect->CurrentLife() >= toAspect->MaxLife())
			{
				int64_t lifePeriodMs = (int64_t)(toAspect->LifeRecoveryPeriod() * 1000.0f);
				PostWorldMessage(we_player_data_changed, to, to, "life", lifePeriodMs);
			}
			toAspect->SetCurrentLife(hp);
			UpdateGo(to);

			// Add exp
			if (!from->Actor()->CanLevelUp())
				return;

			float targetExp = toAspect->ExperienceValue();
			if (targetExp == 0) targetExp = 100.0f;
			float addExp = targetExp / toAspect->MaxLife() * dmg;

			std::string skill = "melee";
			switch (fromInv->GetSelectedSlot())
			{
				case 1: skill = "melee"; break;
				case 2: skill = "ranged"; break;
				case 3:
				case 4:
				{
					Go * item = fromInv->ItemFromLocation((eInventoryLocation)(fromInv->GetSelectedSlot() + 1));
					if (item) skill = item->Magic()->SkillClass();
					break;
				}
			}
			float maxGain = from->Actor()->GetMaxExpGainForLevel(from->Actor()->GetSkillLevel(skill));
			if (addExp > maxGain) addExp = maxGain;

			from->Actor()->AddSkillExp(skill, addExp);
			UpdateGoExp(from, addExp);
		}
		break;

		default: break;
	} /* switch (event) */
}

void Engine :: MessageKnown (Go * go, const WorldMessage & message)
{
	if (go == NULL)
		return;

	// Always send to self first
	go->Send(message);

	// Then send to nearby objects
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

	// Always send to self first
	go->Send(WorldMessage(we_go_status_updated, go, go, ""));

	// Then send to nearby objects
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

	// Always send to self first
	go->Send(WorldMessage(we_go_life_updated, go, go, ""));

	// Then send to nearby objects
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

	// Always send to self first
	go->Send(WorldMessage(we_add_exp, go, go, ""));

	// Then send to nearby objects
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

	// Always send to self first
	go->Send(WorldMessage(we_leveled_up, go, go, ""));

	// Then send to nearby objects
	const GopSet & frustum = go->Frustum();
	for (GopSet::const_iterator iterator = frustum.begin(); iterator != frustum.end(); iterator++)
	{
		(*iterator)->Send (WorldMessage(we_leveled_up, go, (*iterator), data));
	}
}


void Engine::AddGoToRegion(Go *go, const std::string &data)
{
	if (go == NULL)
	{
		Log::Write(Log::Level::ERR, " [AddGoToRegion] Go is null, returning", true);
		return;
	}

	if (data.empty())
	{
		Log::Write(Log::Level::ERR, " [AddGoToRegion] Data is empty, returning.", true);
		return;
	}

	Region *region = nullptr;
	try
	{
		region = g_world.GetRegion(data);
		go->Placement()->SetRegion(data);
		///std::cout << "[AddGoToRegion] Go assigned to region: " << data << std::endl;
	}
	catch (std::exception &e)
	{
		Log::WriteF(Log::Level::ERR, " [AddGoToRegion] Exception getting region: %s", + e.what());
		return;
	}

	GopSet &available = region->Objects();
	available.insert(go);
	std::cout << "[AddGoToRegion] Go inserted into region: " << data << std::endl;

	for (GopSet::iterator it = available.begin(); it != available.end(); ++it)
	{
		Go *object = *it;

		if (AIQuery::IsInRange(go, object, 45.0))
		{
			//std::cout << "[AddGoToRegion] " << go->Goid() << " is in range of " << object->Goid() << std::endl;
			HandleWorldMessage(WorldMessage(we_entered_frustum, go, object, ""));
		}
	}

	/*if (go->HasComponent("player"))
	{
		std::cout << "[AddGoToRegion] Go has 'player' component." << std::endl;
		// Placeholder for future logic
	}*/
}

void Engine::RemoveGoFromRegion(Go *go)
{
	//std::cout << "[RemoveGoFromRegion] Called for Go: " << (go ? go->Goid() : -1) << std::endl;

	if (go == NULL)
	{
		Log::Write(Log::Level::ERR, "[RemoveGoFromRegion] Go is null, returning.", true);
		return;
	}

	Region *region = nullptr;
	try
	{
		region = g_world.GetRegion(go->Placement()->GetRegion());
		//std::cout << "[RemoveGoFromRegion] Region fetched: " << go->Placement()->GetRegion() << std::endl;

		if (go->IsItem())
		{
			go->Placement()->SetRegion("");
			Log::Write(Log::Level::INFO, "[RemoveGoFromRegion] Go is item, region cleared.", true);
		}
	}
	catch (std::exception &e)
	{
		Log::WriteF(Log::Level::ERR, "[RemoveGoFromRegion] Go is item, region cleared. %s", +e.what());
		std::cout << "[RemoveGoFromRegion] Exception: " << e.what() << std::endl;
		return;
	}

	GopSet &available = region->Objects();
	available.erase(go);
	//std::cout << "[RemoveGoFromRegion] Go removed from region." << std::endl;

	const GopSet& frustum_ref = go->Frustum();
	GopSet frustum = frustum_ref; // copy

	for (GopSet::const_iterator it = frustum.begin(); it != frustum.end(); ++it)
	{
		Log::Write(Log::Level::INFO, "[RemoveGoFromRegion] Notifying " + to_string(go->Goid()) + " left frustum of " + to_string((*it)->Goid()), true);
		HandleWorldMessage(WorldMessage(we_left_frustum, go, *it, ""));
	}

	/*if (go->HasComponent("player"))
	{
		std::cout << "[RemoveGoFromRegion] Go has 'player' component." << std::endl;
		// Placeholder for future logic
	}*/
}

void Engine::CalculateFrustums()
{
	set<Region*>& regions = g_world.Regions();
	for (std::set<Region*>::iterator it = regions.begin(); it != regions.end(); ++it)
	{
		Region* region = *it;
		GopSet& objects = region->Objects();
		for (GopSet::iterator i = objects.begin(); i != objects.end(); ++i) // for all GO in region
		{
			Go* object = *i; // each GO
			if (!object->Placement()->IsDirty()) // skip clean placed GOs
				continue;

			// --- Compare GO with every other GO in the same region ---
			for (GopSet::iterator j = objects.begin(); j != objects.end(); ++j)
			{
				Go* other = *j; // each GO
				if (other == object)
					continue;

				//std::cout << "[CalculateFrustums] Checking dirty " << object->Goid() << " vs " << other->Goid() << std::endl;

				bool is_in_frustum = object->Frustum().find(other) != object->Frustum().end();
				bool should_be_in_frustum = AIQuery::IsInRange(other, object, 45.0);

				if (is_in_frustum && !should_be_in_frustum)
				{
					HandleWorldMessage(WorldMessage(we_left_frustum, object, other, ""));
				}
				if (!is_in_frustum && should_be_in_frustum)
				{
					HandleWorldMessage(WorldMessage(we_entered_frustum, object, other, ""));
				}
			}

			object->Placement()->MarkAsClean();
		}
	}
}