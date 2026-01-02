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
#include "GoActor.hpp"
#include "../Engine.hpp"
#include "../helper/Helper.h"
#include "../mysql/MySQL.h"
#include "../Globals.h"

static const vector<float> experience_table = {
		0.0f, 70.0f, 650.0f, 1450.0f, 2350.0f, 3350.0f, 4650.0f, 6650.0f, 9450.0f, 12950.0f,
		17100.0f, 22000.0f, 27800.0f, 34300.0f, 41300.0f, 49191.0f, 58327.0f, 68882.0f,
		81054.0f, 95071.0f, 111190.0f, 129706.0f, 150954.0f, 175317.0f, 203567.0f,
		236338.0f, 274352.0f, 318449.0f, 369600.0f, 428936.0f, 497766.0f, 577609.0f,
		670226.0f, 777662.0f, 902288.0f, 1046855.0f, 1214551.0f, 1409080.0f, 1634732.0f,
		1896489.0f, 2200128.0f, 2552348.0f, 2960924.0f, 3434872.0f, 3984651.0f,
		4622395.0f, 5362179.0f, 6220327.0f, 7215779.0f, 8370504.0f, 9709985.0f,
		11263782.0f, 13066188.0f, 15156978.0f, 17582294.0f, 20395661.0f, 23659167.0f,
		27444833.0f, 31836207.0f, 36930200.0f, 42839232.0f, 49780637.0f, 58283859.0f,
		68700306.0f, 81460453.0f, 97091634.0f, 116239830.0f, 139696370.0f, 168430631.0f,
		203630102.0f, 246749453.0f, 299570658.0f, 364276635.0f, 443541456.0f,
		540640862.0f, 659587634.0f, 805297430.0f, 983791930.0f, 1202447693.0f,
		1470301002.0f, 1798421306.0f, 2200368678.0f, 2692754209.0f, 3295926485.0f,
		4034812522.0f, 4939947918.0f, 6048738778.0f, 7407007581.0f, 9070886865.0f,
		11109138988.0f, 13605997839.0f, 16664649931.0f, 20411498744.0f, 25001388540.0f,
		30624003540.0f, 37511706915.0f, 45949143550.0f, 56285003427.0f, 68946431776.0f,
		84456681504.0f, 103456737421.0f, 126731805919.0f, 155243764829.0f,
		190170914494.0f, 232956672834.0f, 285369226800.0f, 349574605408.0f,
		428226194203.0f, 524574390477.0f, 642600930913.0f, 781081513558566.0f,
		956824851411821.0f, 1172110440282060.0f, 1435835286648100.0f,
		1758898223446500.0f, 2154650321024540.0f, 2639446640557650.0f
};

GoActor::GoActor(Go* go) : GoComponent(go)
{
	m_alignment = aa_neutral;
	m_can_level_up = false;
}

GoActor::GoActor(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
	if (tmplComp == nullptr)
		return;

	const string* f;

	if (f = tmplComp->GetField("alignment"))
	{
		if (FromString(*f, m_alignment) != true) m_alignment = aa_neutral;
	}
	if (f = tmplComp->GetField("can_level_up"))
	{
		if (FromString(*f, m_can_level_up) != true) m_can_level_up = false;
	}
	if (const TemplateComponent* skillComponent = tmplComp->GetSubcomponent("skills"))
	{
		for (std::unordered_map<string, string>::const_iterator it = skillComponent->fields.begin();
			it != skillComponent->fields.end(); ++it)
		{
			const string& fieldName = it->first;
			const string& fieldValue = it->second;

			map<string, Skill*>::iterator iterator = m_skills.find(fieldName);
			if (iterator == m_skills.end())
			{
				Skill* skill = new Skill;
				skill->name = fieldName;
				try { skill->level = std::stof(skillComponent->SkillLevelString(fieldName)); }
				catch (...) { skill->level = 0.0f; }
				try { skill->experience = std::stof(skillComponent->SkillExpString(fieldName)); }
				catch (...) { skill->experience = 0.0f; }

				m_skills[skill->name] = skill;
			}
		}
	}
}

GoActor::GoActor(Go* go, const std::map<std::string, std::string>& r) : GoComponent(go)
{
	string alignmentStr = r.at("alignment");
	if (FromString(alignmentStr, m_alignment) != true) { m_alignment = aa_neutral; }

	m_can_level_up = r.at("can_level_up") == "1";

	{
		Skill* skill = new Skill;
		skill->name = "uber";
		skill->level = std::stof(r.at("skill_uber"));
		skill->experience = std::stod(r.at("skill_uber_exp"));

		m_skills[skill->name] = skill;
	}
	{
		Skill* skill = new Skill;
		skill->name = "strength";
		skill->level = std::stof(r.at("skill_strength"));
		skill->experience = std::stod(r.at("skill_strength_exp"));

		m_skills[skill->name] = skill;
	}
	{
		Skill* skill = new Skill;
		skill->name = "intelligence";
		skill->level = std::stof(r.at("skill_intelligence"));
		skill->experience = std::stod(r.at("skill_intelligence_exp"));

		m_skills[skill->name] = skill;
	}
	{
		Skill* skill = new Skill;
		skill->name = "dexterity";
		skill->level = std::stof(r.at("skill_dexterity"));
		skill->experience = std::stod(r.at("skill_dexterity_exp"));

		m_skills[skill->name] = skill;
	}
	{
		Skill* skill = new Skill;
		skill->name = "melee";
		skill->level = std::stof(r.at("skill_melee"));
		skill->experience = std::stod(r.at("skill_melee_exp"));

		m_skills[skill->name] = skill;
	}
	{
		Skill* skill = new Skill;
		skill->name = "ranged";
		skill->level = std::stof(r.at("skill_ranged"));
		skill->experience = std::stod(r.at("skill_ranged_exp"));

		m_skills[skill->name] = skill;
	}
	{
		Skill* skill = new Skill;
		skill->name = "nature magic";
		skill->level = std::stof(r.at("skill_nature_magic"));
		skill->experience = std::stod(r.at("skill_nature_magic_exp"));

		m_skills[skill->name] = skill;
	}
	{
		Skill* skill = new Skill;
		skill->name = "combat magic";
		skill->level = std::stof(r.at("skill_combat_magic"));
		skill->experience = std::stod(r.at("skill_combat_magic_exp"));

		m_skills[skill->name] = skill;
	}
}

GoActor :: ~GoActor ()
{
	for (map<string, Skill *>::iterator iterator = m_skills.begin(); iterator != m_skills.end(); iterator++)
	{
		delete iterator->second;
	}
}

map<string, Skill*> GoActor::Skills()
{
	return m_skills;
}

void GoActor::Save(MySQL& db)
{
	std::string q =
		"INSERT INTO t_go_actor (go_id, alignment, can_level_up, "
		"skill_uber, skill_uber_exp, "
		"skill_strength, skill_strength_exp, "
		"skill_intelligence, skill_intelligence_exp, "
		"skill_dexterity, skill_dexterity_exp, "
		"skill_melee, skill_melee_exp, "
		"skill_ranged, skill_ranged_exp, "
		"skill_nature_magic, skill_nature_magic_exp, "
		"skill_combat_magic, skill_combat_magic_exp) VALUES (" +
		std::to_string(GetGo()->Goid()) + ", '" + 
		ToString(m_alignment) + "', " +
		std::to_string(m_can_level_up ? 1 : 0) + ", " +
		std::to_string(GetSkillLevel("uber")) + ", " + std::to_string(GetSkillExp("uber")) + ", " +
		std::to_string(GetSkillLevel("strength")) + ", " + std::to_string(GetSkillExp("strength")) + ", " +
		std::to_string(GetSkillLevel("intelligence")) + ", " + std::to_string(GetSkillExp("intelligence")) + ", " +
		std::to_string(GetSkillLevel("dexterity")) + ", " + std::to_string(GetSkillExp("dexterity")) + ", " +
		std::to_string(GetSkillLevel("melee")) + ", " + std::to_string(GetSkillExp("melee")) + ", " +
		std::to_string(GetSkillLevel("ranged")) + ", " + std::to_string(GetSkillExp("ranged")) + ", " +
		std::to_string(GetSkillLevel("nature_magic")) + ", " + std::to_string(GetSkillExp("nature_magic")) + ", " +
		std::to_string(GetSkillLevel("combat_magic")) + ", " + std::to_string(GetSkillExp("combat_magic")) + ") "
		"ON DUPLICATE KEY UPDATE "
		"alignment=VALUES(alignment), can_level_up=VALUES(can_level_up), "
		"skill_uber=VALUES(skill_uber), skill_uber_exp=VALUES(skill_uber_exp), "
		"skill_strength=VALUES(skill_strength), skill_strength_exp=VALUES(skill_strength_exp), "
		"skill_intelligence=VALUES(skill_intelligence), skill_intelligence_exp=VALUES(skill_intelligence_exp), "
		"skill_dexterity=VALUES(skill_dexterity), skill_dexterity_exp=VALUES(skill_dexterity_exp), "
		"skill_melee=VALUES(skill_melee), skill_melee_exp=VALUES(skill_melee_exp), "
		"skill_ranged=VALUES(skill_ranged), skill_ranged_exp=VALUES(skill_ranged_exp), "
		"skill_nature_magic=VALUES(skill_nature_magic), skill_nature_magic_exp=VALUES(skill_nature_magic_exp), "
		"skill_combat_magic=VALUES(skill_combat_magic), skill_combat_magic_exp=VALUES(skill_combat_magic_exp)";

	db.AsyncQuery(q, [](const auto&) {});
}

string GoActor::GetTitle() const
{
	struct TitleGroup {
		vector<string> skills;
		vector<string> maleTitles;
		vector<string> femaleTitles; // optional
	};

	vector<TitleGroup> titleGroups = {
		{{"melee", "ranged", "nature magic", "combat magic"},
		 {"Freelance", "Journeyman", "Adventurer", "Master", "Grand Master", "Grand High SiegeMaster"},
		 {"Freelance", "Journeyman", "Adventurer", "Master", "Grand Master", "Grand High Siegemistress"}},

		{{"melee", "ranged", "combat magic"},
		 {"Mercenary", "Gladiator", "Centurion", "Myrmidon", "Warlord", "Warlord Noble"},
		 {"Mercenary", "Gladiator", "Centurion", "Myrmidon", "Warlady", "Warlady Noble"}},

		{{"melee", "ranged", "nature magic"},
		 {"Cavalier", "Marshal", "Paladin", "Templar", "Arch Templar", "Supreme Templar"}},

		{{"melee", "nature magic", "combat magic"},
		 {"Initiate", "Mystic", "Sage", "Deacon", "Grand Deacon", "Grand High Deacon"},
		 {"Initiate", "Mystic", "Sage", "Deaconess", "Grand Deaconess", "Grand High Deaconess"}},

		{{"ranged", "nature magic", "combat magic"},
		 {"Adept", "Conjurer", "Thaumaturgist", "Evoker", "Senior Evoker", "Lord Evoker"},
		 {"Adept", "Conjurer", "Thaumaturgist", "Evoker", "Senior Evoker", "Lady Evoker"}},

		{{"melee", "ranged"},
		 {"Man-At-Arms", "Skirmisher", "Raider", "Campaigner", "Crusader", "Grand Crusader"},
		 {"Woman-At-Arms", "Skirmisher", "Raider", "Campaigner", "Crusader", "Grand Crusader"}},

		{{"melee", "nature magic"},
		 {"Friar", "Curate", "Druid", "Preserver", "Grand Preserver", "Supreme Preserver"}},

		{{"melee", "combat magic"},
		 {"Combatant", "Duelist", "Dragoon", "Warlock", "Grand Warlock", "Grand High Warlock"},
		 {"Combatant", "Duelist", "Dragoon", "Warwitch", "Grand Warwitch", "Grand High Warwitch"}},

		{{"ranged", "nature magic"},
		 {"Scout", "Forester", "Ranger", "Warder", "Arch Ward", "Supreme High Ward"},
		 {"Scout", "Forester", "Ranger", "Wardess", "Arch Wardess", "Supreme High Wardess"}},

		{{"ranged", "combat magic"},
		 {"Jager", "Conjurer", "Channeler", "Matross", "Master Matross", "Grandmaster Matross"}},

		{{"nature magic", "combat magic"},
		 {"Acolyte", "Shaman", "Scholar", "Magus", "Grand Magus", "Grand High Magus"}},

		{{"melee"},
		 {"Squire", "Soldier", "Warrior", "Knight", "Champion", "Grand Champion"}},

		{{"ranged"},
		 {"Bowyer", "Archer", "Marksman", "Sharpshooter", "Master Sharpshooter", "Grandmaster Sharpshooter"}},

		{{"nature magic"},
		 {"Apprentice", "Theurgist", "Magician", "Grand Mage", "Arch Mage", "Supreme Arch Mage"}},

		{{"combat magic"},
		 {"Savant", "Hedge Wizard", "Wizard", "Sorcerer", "Grand Sorcerer", "Grand High Sorcerer"},
		 {"Savant", "Hedge Wizard", "Wizard", "Sorceress", "Grand Sorceress", "Grand High Sorceress"}}
	};

	auto getBracket = [](float level) {
		if (level >= 100) return 5;
		if (level >= 50)  return 4;
		if (level >= 20)  return 3;
		if (level >= 11)  return 2;
		if (level >= 5)   return 1;
		if (level >= 1)   return 0;
		return -1;
		};

	int gender = 0; // TODO add gender getter GetGender(); // 0 = male, 1 = female

	for (const auto& group : titleGroups) {
		bool qualifies = true;
		int minBracket = 6;

		for (const string& skill : group.skills) {
			float level = GetSkillLevel(skill.c_str());
			int bracket = getBracket(level);
			if (bracket == -1) {
				qualifies = false;
				break;
			}
			minBracket = min(minBracket, bracket);
		}

		if (qualifies) {
			if (gender == 1 && group.femaleTitles.size() == 6)
				return group.femaleTitles[minBracket];
			else
				return group.maleTitles[minBracket];
		}
	}

	return "Noob";
}

eActorAlignment GoActor :: Alignment () const
{
	return m_alignment;
}

bool GoActor::CanLevelUp() const
{
	return m_can_level_up;
}

void GoActor::SetCanLevelUp(bool can)
{
	m_can_level_up = can;
}


float GoActor :: GetLevelFromXP(double xp) {
    const auto& table = experience_table;
    size_t level = 0;

    // Find the highest level such that required XP is <= current XP
    while (level + 1 < table.size() && table[level + 1] <= xp) {
        ++level;
    }

    // If max level reached or exact match
    if (level + 1 >= table.size() || table[level] == xp) {
        return static_cast<float>(level);
    }

    // Interpolate fractional part
    float xp_start = table[level];
	float xp_end   = table[level + 1];
    float fraction = static_cast<float>(xp - xp_start) / (xp_end - xp_start);

    return static_cast<float>(level) + fraction;
}

double GoActor :: GetXPFromLevel(float level) {
    const auto& table = experience_table;
    int baseLevel = static_cast<int>(floor(level));

    if (baseLevel >= static_cast<int>(table.size()) - 1) {
        return table.back(); // Clamp to max XP in table
    }

    float fraction = level - baseLevel;
	float xp_start = table[baseLevel];
	float xp_end = table[baseLevel + 1];

    return (xp_start + fraction * (xp_end - xp_start));
}

double  GoActor :: GetMaxExpGainForLevel(float level)
{
    static const std::vector<double> maxExpPerLevel = {
        7.00f,14.50f,20.00f,22.50f,25.00f,32.50f,50.00f,70.00f,87.50f,103.75f,122.50f,145.00f,162.50f,175.00f,197.28f,228.40f,
        263.88f,304.30f,350.43f,402.98f,462.90f,531.20f,609.08f,706.25f,819.28f,950.35f,1102.42f,1278.78f,1483.40f,1720.75f,
        1996.08f,2315.43f,2685.90f,3115.65f,3614.18f,4192.40f,4863.23f,5641.30f,6543.93f,7590.98f,8805.50f,10214.40f,11848.70f,
        13744.48f,15943.60f,18494.60f,21453.70f,24886.30f,28868.12f,33487.03f,38844.93f,45060.15f,52269.75f,60632.90f,70334.18f,
        81587.65f,94641.65f,109784.35f,127349.83f,147725.80f,173535.12f,212580.55f,260411.18f,319003.68f,390779.53f,478704.90f,
        586413.50f,718356.53f,879986.78f,1077983.78f,1320530.12f,1617649.43f,1981620.53f,2427485.15f,2973669.30f,3642744.90f,
        4462362.50f,5466394.08f,6696332.73f,8203007.60f,10048684.30f,12309638.28f,15079306.90f,18472150.93f,22628384.90f,
        27719771.50f,33956720.08f,41596982.10f,50956303.08f,62421471.28f,76466302.30f,93671220.33f,114747244.90f,140565375.00f,
        172192584.38f,210935915.88f,258396496.93f,316535708.73f,387756243.20f,475001397.93f,581876712.45f,712798972.75f,
        873178741.62f,1069643958.50f,1310313849.15f,1605134465.20f,1966289719.88f,2408704906.85f,2950663510.90f,3614562800.83f,
        4427839431.05f,5424103303.03f,6644526546.18f,8139545019.10f,9970942648.38f,12214404744.28f,14962645811.73f,
        18329241119.38f,22453320371.23f,27505317454.78f,33694013882.08f,41275167005.53f,50562079581.80f,61938547487.70f,
        75874720672.43f,92946532823.70f,113859502709.08f,139477890818.58f,170860416252.78f,209304009909.68f,256397412139.30f,
        314086829870.67f,384756366591.58f,471326549074.68f,577375022616.47f,707284402705.20f,866423393313.85f,1061368656809.48f,
        1300176604591.60f,1592716340624.70f,1951077517265.28f,2390069958649.98f,2927835699346.20f,3586598731699.10f,
        4393583446331.38f,5382139721755.98f,6593121159151.00f,8076573419960.00f,9893802439451.00f
    };

    int index = static_cast<int>(floor(level));

    // Clamp index to array bounds
    if (index < 0)
        index = 0;
    else if (index >= static_cast<int>(maxExpPerLevel.size()))
        index = static_cast<int>(maxExpPerLevel.size()) - 1;

    return maxExpPerLevel[index];
}
void GoActor :: ChangeSkillLevel (const string & skill, float delta)
{
	map<string, Skill *>::iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		iterator->second->level =+ delta;
		if (iterator->second->level < 0.0)
		{
			iterator->second->level = 0.0;
		}
	}
}

bool GoActor :: HasSkill (const string & skill) const
{
	map<string, Skill *>::const_iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		return true;
	}
	return false;
}

float GoActor :: GetSkillLevel (const string & skill) const
{
	map<string, Skill *>::const_iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		return iterator->second->level;
	}
	
	return 0.0;
}

double GoActor :: GetSkillExp (const string & skill) const
{
	map<string, Skill *>::const_iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		return iterator->second->experience;
	}
	return 0;
}

float GoActor :: HighestSkillLevel () const
{
	float highest = 0.0;
	for (map<string, Skill *>::const_iterator iterator = m_skills.begin(); iterator != m_skills.end(); iterator++)
	{
		if (iterator->second->level > highest)
		{
			return iterator->second->level;
		}
	}
	
	return highest;
}

void GoActor :: SetAlignment (eActorAlignment alignment)
{
	m_alignment = alignment;
}

void GoActor :: SetSkillLevel (const string & skill, float value)
{
	map<string, Skill *>::iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		iterator->second->level = value;
		double exp = GetXPFromLevel(value);
		iterator->second->experience = exp;
	}
}

void GoActor :: SetSkillExp (const string & skill, double value)
{
	map<string, Skill *>::iterator iterator = m_skills.find (skill);
	if (iterator != m_skills.end())
	{
		iterator->second->experience = value;
		float level = GetLevelFromXP(value);
		iterator->second->level = level;
	}
}

void GoActor::AddSkillExp(const string& skill, double value)
{
	auto it = m_skills.find(skill);
	if (it == m_skills.end())
		return;

	// Special melee weapon check (unchanged logic)
	if (skill == "melee" && !GetGo()->Inventory()->IsMeleeWeaponEquipped())
	{
		AddSkillExp("uber", value);
		return;
	}

	Skill* skillPtr = it->second;

	/* ------------------------------------------------------------
	   1. AWARD SKILL XP (NORMAL SKILL BEHAVIOR)
	   ------------------------------------------------------------ */

	double oldXP = skillPtr->experience;
	float oldLevel = skillPtr->level;

	skillPtr->experience += value;

	if (skillPtr->experience > experience_table.back())
		skillPtr->experience = experience_table.back();

	skillPtr->level = GetLevelFromXP(skillPtr->experience);

	if (std::floor(skillPtr->level) > std::floor(oldLevel))
	{
		if (skill != "uber")
		{
			g_engine.UpdateGoLvlup(GetGo(), skill);
		}
	}

	/* ------------------------------------------------------------
	   2. PRIMARY SKILLS DRIVE ATTRIBUTES + UBER XP
	   ------------------------------------------------------------ */

	bool isPrimary =
		skill == "melee" ||
		skill == "ranged" ||
		skill == "nature magic" ||
		skill == "combat magic";

	if (!isPrimary)
		return;

	/* ------------------------------------------------------------
	   3. AWARD UBER XP (ONCE, NO RECURSION)
	   ------------------------------------------------------------ */

	Skill* uber = m_skills["uber"];

	double uberOldXP = uber->experience;
	float uberOldLevel = uber->level;

	uber->experience += value;

	if (uber->experience > experience_table.back())
		uber->experience = experience_table.back();

	uber->level = GetLevelFromXP(uber->experience);

	/* ------------------------------------------------------------
	   4. COMPUTE UBER FACTOR (FRACTION OF ONE ATTRIBUTE POINT)
	   ------------------------------------------------------------ */

	float uberFloor = std::floor(uberOldLevel);

	double xpCurrent = GetXPFromLevel(uberFloor);
	double xpNext = GetXPFromLevel(uberFloor + 1.0f);

	double xpToNextUber = xpNext - xpCurrent;

	if (xpToNextUber <= 0.0f)
		return;

	double uber_factor = value / xpToNextUber;

	/* ------------------------------------------------------------
	   5. ATTRIBUTE INFLUENCE WEIGHTS (DS1 TABLE)
	   ------------------------------------------------------------ */

	double strInf = 0.f, dexInf = 0.f, intInf = 0.f;

	if (skill == "melee") {
		strInf = 0.64f; dexInf = 0.27f; intInf = 0.09f;
	}
	else if (skill == "ranged") {
		strInf = 0.25f; dexInf = 0.62f; intInf = 0.13f;
	}
	else if (skill == "nature magic") {
		strInf = 0.09f; dexInf = 0.18f; intInf = 0.73f;
	}
	else if (skill == "combat magic") {
		strInf = 0.13f; dexInf = 0.17f; intInf = 0.70f;
	}

	/* ------------------------------------------------------------
	   6. APPLY ATTRIBUTE GROWTH (FLOAT LEVELS, NO XP)
	   ------------------------------------------------------------ */

	auto applyAttribute = [&](const string& attr, double delta)
		{
			Skill* a = m_skills[attr];
			float old = a->level;
			a->level += delta;
			a->experience = GetXPFromLevel(a->level);

			if (std::floor(old) != std::floor(a->level))
			{
				g_engine.UpdateGoLvlup(GetGo(), attr);
			}
		};

	applyAttribute("strength", uber_factor * strInf);
	applyAttribute("dexterity", uber_factor * dexInf);
	applyAttribute("intelligence", uber_factor * intInf);
}