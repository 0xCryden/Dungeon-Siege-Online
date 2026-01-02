#include "../Common.h"

#include "Go.hpp"
#include "../utils/AIQuery.h"
#include "GoAttack.hpp"
#include "../Engine.hpp"
#include "../helper/Helper.h"


GoAttack :: GoAttack (Go * go) : GoComponent (go)
{
}

GoAttack::GoAttack(Go* go, const TemplateComponent* tmplComp) : GoComponent (go)
{
	if (tmplComp == nullptr)
		return;

	const string* f;

	if (f = tmplComp->GetField("attack_range")) { try { m_attack_range = std::stof(*f); } catch (...) { m_attack_range = 1.0f; } }
	if (f = tmplComp->GetField("critical_hit_chance")) { try { m_critical_hit_chance = std::stof(*f); } catch (...) { m_critical_hit_chance = 0.0f; } }
	if (f = tmplComp->GetField("damage_max")) { try { m_damage_max = std::stof(*f); } catch (...) { m_damage_max = 0.0f; } }
	if (f = tmplComp->GetField("damage_min")) { try { m_damage_min = std::stof(*f); } catch (...) { m_damage_min = 0.0f; } }
	if (f = tmplComp->GetField("is_two_handed")) { if (FromString(*f, m_two_handed) != true) m_two_handed = false; }
	if (f = tmplComp->GetField("reload_delay")) { try { m_reload_delay = static_cast<uint64_t>(std::stoi(*f)); } catch (...) { m_reload_delay = 0; } }
	if (f = tmplComp->GetField("attack_class")) { if (FromString(*f, m_attack_class) != true) m_attack_class = ac_beastfu; }
}
GoAttack::GoAttack(Go* go, const std::map<std::string, std::string>& r) : GoComponent(go)
{
	m_attack_range = std::stof(r.at("attack_range"));
	m_critical_hit_chance = std::stof(r.at("critical_hit_chance"));

	m_damage_min = std::stof(r.at("damage_min"));
	m_damage_max = std::stof(r.at("damage_max"));

	m_two_handed = r.at("is_two_handed") == "1";
	m_reload_delay = std::stoull(r.at("reload_delay"));

	m_attack_class = acStringToNum(r.at("attack_class"));
}

void GoAttack::Save(MySQL& db)
{
	std::string q =
		"INSERT INTO t_go_attack (go_id, attack_range, critical_hit_chance, "
		"damage_min, damage_max, is_two_handed, reload_delay, attack_class) VALUES (" +
		std::to_string(GetGo()->Goid()) + ", " +
		std::to_string(m_attack_range) + ", " +
		std::to_string(m_critical_hit_chance) + ", " +
		std::to_string(m_damage_min) + ", " +
		std::to_string(m_damage_max) + ", " +
		std::to_string(m_two_handed ? 1 : 0) + ", " +
		std::to_string(m_reload_delay) + ", '" +
		ToString(m_attack_class) + "') "
		"ON DUPLICATE KEY UPDATE "
		"attack_range=VALUES(attack_range), "
		"critical_hit_chance=VALUES(critical_hit_chance), "
		"damage_min=VALUES(damage_min), "
		"damage_max=VALUES(damage_max), "
		"is_two_handed=VALUES(is_two_handed), "
		"reload_delay=VALUES(reload_delay), "
		"attack_class=VALUES(attack_class)";

	db.AsyncQuery(q, [](const auto&) {});
}

float GoAttack :: AttackRange () const
{
	return m_attack_range;
}

float GoAttack :: CriticalHitChance () const
{
	return m_critical_hit_chance;
}

float GoAttack :: DamageMax () const
{
	return m_damage_max;
}

float GoAttack :: DamageMin () const
{
	return m_damage_min;
}

bool GoAttack :: IsInProjectileRange (const Go * go)
{
	return AIQuery::IsInRange (m_go, go, m_attack_range);
}

bool GoAttack :: IsTwoHanded () const
{
	return m_two_handed;
}

uint64_t GoAttack :: ReloadDelay () const
{
	return m_reload_delay;
}

eAttackClass GoAttack :: AttackClass () const
{
	return m_attack_class;
}

bool GoAttack :: IsMelee()
{
	switch (m_attack_class)
	{
		case ac_beastfu:
		case ac_axe:
		case ac_club:
		case ac_dagger:
		case ac_hammer:
		case ac_mace:
		case ac_staff:
		case ac_sword:
		{
			return true;
		}
		break;

		default:
			return false;
	}
}

bool GoAttack :: IsRanged()
{
	switch (m_attack_class)
	{
		case ac_bow:
		case ac_minigun:
		case ac_arrow:
		case ac_bolt:
		{
			return true;
		}
		break;

		default:
			return false;
	}
}

bool GoAttack :: IsMagic()
{
	switch (m_attack_class)
	{
		case ac_combat_magic:
		case ac_nature_magic:
		{
			return true;
		}
		break;

		default:
			return false;
	}
}

int GoAttack::CalcHitType(Go * target, const string & skill, GoMagic * magic)
{
	if (!GetGo() || !target)
		return 0; // HT_MISS

	GoActor* atkActor = GetGo()->Actor();
	GoActor* tgtActor = target->Actor();
	if (!atkActor || !tgtActor)
		return 0;

	// TODO add dodge chance
	/*GoDefend* def = target->Defend();
	if (def)
	{
	    float dodgeGeneral = def->GetChanceToDodgeHit();
	    float dodgeSpecific = 0.0f;

	    if (skill == "melee")
	        dodgeSpecific = def->GetChanceToDodgeHitMelee();
	    else if (skill == "ranged")
	        dodgeSpecific = def->GetChanceToDodgeHitRanged();

	    if ((float)(rand() % 100) < dodgeGeneral || (float)(rand() % 100) < dodgeSpecific)
	        return 0; // HT_MISS
	}*/

	if (skill == "melee")
	{
		// --- RATING CALC ---
		const float skill_scalar         = 0.45f;
		const float dex_scalar           = 0.55f;
		const float int_scalar           = 0.15f;
		const float base_hit_chance      = 50.0f;
		const float attacker_diff_scalar = 2.1f;
		const float victim_diff_scalar   = 2.1f;
		const float attacker_hit_cap     = 95.0f;
		const float defender_hit_cap     = 5.0f;

		float atk_skill = 0.0f;
		if (GetGo()->Inventory()->IsMeleeWeaponEquipped())
		{
			atk_skill = atkActor->GetSkillLevel(skill);
		}
		float atk_dex   = atkActor->GetSkillLevel("dexterity") + 10;
		float atk_int   = atkActor->GetSkillLevel("intelligence") + 10;

		float atk_rating = (skill_scalar * atk_skill) +
						   (dex_scalar * atk_dex) +
						   (int_scalar * atk_int);

		// victim skill only if it should be considered
		float def_rating = 0.0f;
		float def_skill = 0.0f;
		if ((target->Inventory()->IsMeleeWeaponEquipped()) && (GetGo()->Inventory()->IsMeleeWeaponEquipped() == true))
		{
			def_skill = tgtActor->GetSkillLevel(skill);
		}
		float def_dex   = tgtActor->GetSkillLevel("dexterity") + 10;
		float def_int   = tgtActor->GetSkillLevel("intelligence") + 10;

		def_rating = (skill_scalar * def_skill) +
					 (dex_scalar * def_dex) +
					 (int_scalar * def_int);


		float rating_diff = atk_rating - def_rating;
		float hit_chance = base_hit_chance;

		if (rating_diff > 0)
			hit_chance += rating_diff * attacker_diff_scalar;
		else
			hit_chance += rating_diff * victim_diff_scalar;

		if (hit_chance > attacker_hit_cap) hit_chance = attacker_hit_cap;
		if (hit_chance < defender_hit_cap) hit_chance = defender_hit_cap;

		// TODO add bonus hit chances
		// --- BONUSES ---
		/*float baseChance = hit_chance;
		float bonus = 0.0f;
		// e.g. Load bonus from a table or static map:
		// GetHitBonusAgainstTemplate(target->TemplateName(), &bonus);
		hit_chance += bonus * baseChance;

		hit_chance += GetGo()->Attack()->ChanceToHitBonus() * baseChance;

		if (skill == "melee")
			hit_chance += 0.01f * GetGo()->Attack()->ChanceToHitBonusMelee() * baseChance;
		else if (skill == "ranged")
			hit_chance += 0.01f * GetGo()->Attack()->ChanceToHitBonusRanged() * baseChance;*/

		// Cap again
		if (hit_chance > attacker_hit_cap) hit_chance = attacker_hit_cap;

		// --- ROLL FOR HIT ---
		if (((float)(rand() % 10000) / 100.0f) > hit_chance)
			return 0; // HT_MISS

		// --- ROLL FOR CRIT ---
		if (((float)(rand() % 100)) < GetGo()->Attack()->CriticalHitChance())
			return 2; // HT_CRIT

		return 1; // HT_HIT
	}
	else // skill == "ranged"
	{
		// --- AIMING ERROR CALC ---
		const float error_scalar = 4.0f;
		const float dex_scalar   = 0.35f;
		const float int_scalar   = 0.10f;
		const float skill_scalar = 0.55f;

		float dex   = atkActor->GetSkillLevel("dexterity") + 10;
		float intel = atkActor->GetSkillLevel("intelligence") + 10;
		float skill_level = atkActor->GetSkillLevel(skill);

		// Core error calculation
		float val = (dex_scalar * dex + int_scalar * intel + skill_scalar * skill_level) / 14.7f;
		float aiming_error_deg = error_scalar * ((100.0f - atanf(val) * 63.0f) / 100.0f);

		// Random aim offset (simulate real deviation)
		float offset_x = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * aiming_error_deg;
		float offset_y = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * aiming_error_deg;

		// Use a fixed tolerance for "acceptable" error to count as hit
		const float max_allowed_angle = 4.0f; // degrees

		if (fabs(offset_x) > max_allowed_angle || fabs(offset_y) > max_allowed_angle)
			return 0; // HT_MISS

		// --- RATING CALC FOR RANGED ---
		const float base_hit_chance      = 50.0f;
		const float attacker_diff_scalar = 2.1f;
		const float victim_diff_scalar   = 2.1f;
		const float attacker_hit_cap     = 95.0f;
		const float defender_hit_cap     = 5.0f;

		float atk_rating = (skill_scalar * skill_level) +
						   (dex_scalar * dex) +
						   (int_scalar * intel);

		float def_skill = tgtActor->GetSkillLevel(skill);
		float def_dex   = tgtActor->GetSkillLevel("dexterity") + 10;
		float def_int   = tgtActor->GetSkillLevel("intelligence") + 10;

		float def_rating = (skill_scalar * def_skill) +
						   (dex_scalar * def_dex) +
						   (int_scalar * def_int);

		float rating_diff = atk_rating - def_rating;
		float hit_chance = base_hit_chance;

		if (rating_diff > 0)
			hit_chance += rating_diff * attacker_diff_scalar;
		else
			hit_chance += rating_diff * victim_diff_scalar;

		if (hit_chance > attacker_hit_cap) hit_chance = attacker_hit_cap;
		if (hit_chance < defender_hit_cap) hit_chance = defender_hit_cap;

		// TODO add bonus hit chances
		// --- BONUSES ---
		/*float baseChance = hit_chance;
		float bonus = 0.0f;
		// e.g. Load bonus from a table or static map:
		// GetHitBonusAgainstTemplate(target->TemplateName(), &bonus);
		hit_chance += bonus * baseChance;

		hit_chance += GetGo()->Attack()->ChanceToHitBonus() * baseChance;

		if (skill == "melee")
			hit_chance += 0.01f * GetGo()->Attack()->ChanceToHitBonusMelee() * baseChance;
		else if (skill == "ranged")
			hit_chance += 0.01f * GetGo()->Attack()->ChanceToHitBonusRanged() * baseChance;*/

		// Cap again
		if (hit_chance > attacker_hit_cap) hit_chance = attacker_hit_cap;

		// --- ROLL FOR HIT ---
		if (((float)(rand() % 10000) / 100.0f) > hit_chance)
			return 0; // HT_MISS

		// --- ROLL FOR CRIT ---
		if (((float)(rand() % 100)) < GetGo()->Attack()->CriticalHitChance())
			return 2; // HT_CRIT

		return 1; // HT_HIT
	}
}

inline float ClampRelative(float value, float reference, float minRatio = 0.05f, float maxRatio = 0.95f)
{
	float lowerBound = reference * minRatio;
	float upperBound = reference * maxRatio;
	if (value < lowerBound) return lowerBound;
	if (value > upperBound) return upperBound;
	return value;
}

float GoAttack::CalcDamage(Go* target, const string& skill, int hitType, GoMagic* magic)
{
	bool log = false;

	//return rand() % 10 + 1;
	float damage = (float)(rand() % 10 + 1);
	float min_damage = 0.0f;
	float max_damage = 0.0f;

	Go* attacker = GetGo();
	if (log) cout << "[CalcDamage] Got attacker: " << (attacker ? "valid" : "null") << endl;

	if (!target || !attacker) {
		if (log) cout << "[CalcDamage] Target or attacker is null. Exiting early." << endl;
		return 0.0f;
	}

	//GoAttack* atkComp = attacker->Attack();
	GoActor* actorComp = attacker->Actor();
	if (log) cout << "[CalcDamage] Got actor component: " << (actorComp ? "valid" : "null") << endl;

	Go* weapon = NULL; // Default: unarmed
	//if (log) cout << "[CalcDamage] Default weapon set to attacker." << endl;

	// Try to get the equipped weapon
	if (skill == "melee")
	{
		Go* equipped = attacker->Inventory()->GetEquipped(es_weapon_hand);
		if (log) cout << "[CalcDamage] Equipped melee weapon: " << (equipped ? "valid" : "null") << endl;
		if (equipped && equipped->HasAttack())
		{
			weapon = equipped;
			if (log) cout << "[CalcDamage] Melee weapon updated to equipped." << endl;
		}
	}
	else if (skill == "ranged")
	{
		Go* equipped = attacker->Inventory()->GetEquipped(es_shield_hand);
		if (log) cout << "[CalcDamage] Equipped ranged weapon: " << (equipped ? "valid" : "null") << endl;
		if (equipped && equipped->HasAttack())
		{
			weapon = equipped;
			if (log) cout << "[CalcDamage] Ranged weapon updated to equipped." << endl;
		}
	}

	// Base weapon damage
	if (weapon && weapon->HasAttack())
	{
		min_damage = weapon->Attack()->DamageMin();
		max_damage = weapon->Attack()->DamageMax();
		if (log) cout << "[CalcDamage] Weapon base damage: min=" << min_damage << ", max=" << max_damage << endl;
	}
	/*if (attacker != weapon && skill == "melee")
	{
		min_damage += attacker->Attack()->DamageMin();
		max_damage += attacker->Attack()->DamageMax();
		if (log) cout << "[CalcDamage] Added attacker's damage: min=" << min_damage << ", max=" << max_damage << endl;
	}*/

	// Damage range calculation based on weapon and skill
	if (skill == "melee")
	{
		// Strength bonus
		if (actorComp && actorComp->CanLevelUp())
		{
			float strength = (floorf(actorComp->GetSkillLevel("strength")) + 10.0f);
			if (log) cout << "[CalcDamage] Strength: " << strength << endl;

			/*int min_str_bonus = (strength >= 10.0f)
				? static_cast<int>(2.0f + ((strength - 10.0f) * 2.07f))
				: 2;

			int max_str_bonus = (strength >= 10.0f)
				? static_cast<int>(4.0f + ((strength - 10.0f) * 2.13f))
				: 4;*/
			min_damage += 2 + (1.397f * (strength - 10));
			max_damage += 4 + (1.603f * (strength - 10));

			//min_damage += atkComp->DamageBonusMinMelee();
			//max_damage += atkComp->DamageBonusMaxMelee();
			if (log) cout << "[CalcDamage] After strength bonus: min=" << min_damage << ", max=" << max_damage << endl;
		}
		else
		{
			//min_damage += atkComp->DamageBonusMinMelee();
			//max_damage += atkComp->DamageBonusMaxMelee();
		}
	}
	else if (skill == "ranged")
	{
		//min_damage += atkComp->DamageBonusMinRanged();
		//max_damage += atkComp->DamageBonusMaxRanged();
		if (log) cout << "[CalcDamage] Skipping ranged bonus (commented out)." << endl;
	}
	else if (skill == "nature magic" || skill == "combat magic")
	{
		/*min_damage = weapon->Magic()->DamageMin();
		max_damage = weapon->Magic()->DamageMax();

		if (magic && magic->MagicClass() == MC_COMBAT_MAGIC)
		{
			min_damage += atkComp->GetDamageBonusMinCMagic();
			max_damage += atkComp->GetDamageBonusMaxCMagic();
		}
		else
		{
			min_damage += atkComp->GetDamageBonusMinNMagic();
			max_damage += atkComp->GetDamageBonusMaxNMagic();
		}*/
		if (log) cout << "[CalcDamage] Skipping magic damage logic (commented out)." << endl;
	}

	// Clamp and validate damage range
	min_damage = max(1.0f, min_damage);
	max_damage = max(min_damage, max_damage);
	if (log) cout << "[CalcDamage] Clamped damage range: min=" << min_damage << ", max=" << max_damage << endl;

	if (!g_engine.IsPlayer(attacker))
	{
		min_damage = attacker->Attack()->DamageMin();
		max_damage = attacker->Attack()->DamageMax();
	}

	// Random roll in rangefloat range = max_damage - min_damage;
	float base_damage = 0.0f;
	float range = max_damage - min_damage;
	if (range <= 0.0f) {
		base_damage = min_damage;   // or max_damage, same when equal
	}
	else {
		base_damage = (rand() % (int)range) + min_damage;
	}//min_damage + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (max_damage - min_damage);
	if (log) cout << "[CalcDamage] Rolled base damage: " << base_damage << endl;

	// Pre-defense backup for relative clamp
	float preDefense = base_damage;

	// Final scaling with skill level and defense
	float skillLevel = actorComp->GetSkillLevel(skill);
	float defense = target->Defend()->GetTotalDefense();
	if (log) cout << "[CalcDamage] Skill level: " << skillLevel << ", Defense: " << defense << endl;

	if (skill == "melee")
	{
		damage = ((base_damage * (skillLevel + 1.552f)) * 4.059f) / defense;
	}
	else if (skill == "ranged")
	{
		damage = ((base_damage * (skillLevel + 1.949f)) * 3.232f) / defense;
	}
	else if (skill == "nature magic" || skill == "combat magic")
	{
		damage = ((base_damage * (skillLevel + 2.177f)) * 2.894f) / defense;
	}
	else
	{
		damage = base_damage; // fallback
		if (log) cout << "[CalcDamage] Skill type unrecognized, using fallback base damage." << endl;
	}

	//cout << "Calculated damage: " << damage << endl;
	if (log) cout << "[CalcDamage] Damage before clamp: " << damage << endl;

	damage = ClampRelative(damage, preDefense);
	if (log) cout << "[CalcDamage] Final damage after ClampRelative: " << damage << endl;

	/*// === LIFE/MANA STEAL (melee only) ===
	if (skill == "melee") {
		damage += LifeSteal(); // heal amount is added to outgoing damage
		damage += ManaSteal();
	}*/

	return damage;
}


uint64_t GoAttack::CalcHitTime()
{
	const uint64_t defaultDelay = 1250;

	Go* go = GetGo();
	if (!go)
		return defaultDelay;

	GoInventory* inv = go->Inventory();
	if (!inv)
		return defaultDelay;

	uint64_t delayTotal = defaultDelay;

	if (g_engine.IsPlayer(go))
	{
		// Check offhand/shield hand first
		Go* shieldHand = inv->GetEquipped(es_shield_hand);
		if (shieldHand)
		{
			GoAttack* atk = shieldHand->HasAttack() ? shieldHand->Attack() : nullptr;
			GoAttack* goAtk = go->HasAttack() ? go->Attack() : nullptr;

			if (atk && goAtk && atk->IsTwoHanded()) // FIXME: change this to AC_BOW condition if needed
			{
				delayTotal = static_cast<uint64_t>((atk->ReloadDelay() + goAtk->ReloadDelay()) * 1000.0f);
				return delayTotal;
			}
		}

		// Now check main weapon
		Go* weapon = inv->GetEquipped(es_weapon_hand);
		if (!weapon || !weapon->HasAttack())
		{
			//shield but no main weapon
			if (inv->GetEquipped(es_shield_hand) && inv->GetEquipped(es_shield_hand)->HasDefend())
			{
				delayTotal = 1000;
				return delayTotal;
			}

			return delayTotal;
		}

		GoAttack* weaponAtk = weapon->Attack();
		if (!weaponAtk)
			return delayTotal;

		if (!go->HasAspect() || !go->Aspect())
		{
			cout << "CalcHitTime attacker aspect not found" << endl;
			Log::WriteF(Log::Level::WARNING, "CalcHitTime attacker aspect not found");
			return defaultDelay;
		}

		eAttackClass atkCls = weaponAtk->AttackClass();

		switch (atkCls)
		{
			case ac_beastfu:
			{
				delayTotal = 1250;
			}
			break;

			case ac_minigun:
			{
				delayTotal = 125;
			}
			break;

			case ac_staff:
			case ac_axe:
			case ac_club:
			case ac_dagger:
			case ac_hammer:
			case ac_mace:
			case ac_sword:
			{
				if (!weaponAtk->IsTwoHanded())
				{
					// Possibly apply a race-based modifier here
					delayTotal = 830;

					/*if (go->Inventory()->GetEquipped(es_shield_hand) != NULL)
					{
						delayTotal = 830;
					}*/
				}
				else
				{
					delayTotal = 1250;
					// Two-handed sword logic
				}
			}
			break;

			case ac_bow:
			{
				delayTotal = 1000;
			}
			break;

			default:
				//if (weaponAtk->IsTwoHanded())
				{
					delayTotal = 1250;
					// Generic 2H delay logic
				}
				break;
		}
	}
	else // Non-player attacker
	{
		if (!go->HasAttack())
			return defaultDelay;

		GoAttack* atk = go->Attack();
		if (!atk)
			return defaultDelay;

		delayTotal = static_cast<uint64_t>(atk->ReloadDelay() * 1000.0f);

	}

	if (go->HasInventory() && go->Inventory()->GetEquipped(es_weapon_hand))
	{
		delayTotal += go->Inventory()->GetEquipped(es_weapon_hand)->Attack()->ReloadDelay();
	}

	return delayTotal;
}
