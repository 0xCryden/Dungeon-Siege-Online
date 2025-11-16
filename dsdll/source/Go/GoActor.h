#ifndef _GOACTOR_H_
#define _GOACTOR_H_

#include <fstream>
#include <cstring>     // stricmp
#include <cstdio>      // sprintf
#include <cstdlib>     // atof

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

static std::map<std::string, std::string> g_CustomVars;
static std::string g_LastReadValue; // holds value for ReadCustomVar return

struct SkillEntry
{
	const char* name;			// X
	DWORD unknown1;				// padding
	const char* screen_name;	// x
	DWORD unknown2;				//padding

	float str_influence;		//x
	float dex_influence;		//x
	float int_influence;		//x

	double xp;
	double xp_next_level;

	float base_level;			//x
	float current_level;	
	float level_bias;
	float level_bias_modifier;
	float max_level;			//x

	void* owner;				//x
};

static const float rawMaxExpPerLevel[] = {
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

static const std::vector<float> maxExpPerLevel(
	rawMaxExpPerLevel,
	rawMaxExpPerLevel + sizeof(rawMaxExpPerLevel) / sizeof(rawMaxExpPerLevel[0])
);

class GoActor
{
	FEX float GetMaxExpPerHit(int level) 
	{
		if (level <= 0) return 0.0f;
		if ((size_t)level > maxExpPerLevel.size()) return maxExpPerLevel.back();
		return maxExpPerLevel[level - 1];
	}

	FEX void SetBaseSkillLevel(const char* skill, float value)
	{
		SkillEntry* s = FindSkill(skill);
		if (s) s->base_level = value;
	}

	FEX void ChangeBaseSkillLevel(const char* skill, float delta)
	{
		SkillEntry* s = FindSkill(skill);
		if (s)
		{
			s->base_level += delta;
			if (s->base_level < 0) s->base_level = 0;
		}
	}

	FEX float GetBaseSkillLevel(const char* skill) const
	{
		SkillEntry* s = FindSkill(skill);
		return s ? s->base_level : 0.0f;
	}

	FEX float GetSkillExperience(const char* skill) const
	{
		SkillEntry* s = FindSkill(skill);
		return s ? static_cast<float>(s->xp) : 0.0f;
	}

	FEX const char* GetSkillExpAsString(const char* skill)
	{
		static char buffer[64];
		sprintf(buffer, "%.0f", GetSkillExperience(skill));
		return buffer;
	}

	FEX void SetSkillExperience(const char* skill, float xp)
	{
		SkillEntry* s = FindSkill(skill);
		if (s) {
			s->xp = static_cast<double>(xp);
		}
	}

	FEX void SetSkillExpFromString(const char* skill, const char* xpStr)
	{
		float xp = static_cast<float>(atof(xpStr));
		SetSkillExperience(skill, xp);
	}

	/// Dumps all skill-related data for analysis
	FEX void DebugPrintSkills()
	{
		DWORD* addr = (DWORD*)this;
		SkillEntry* skillEnd = (SkillEntry*)(addr[0x07]);

		for (SkillEntry* skillEntry = (SkillEntry*)(addr[0x06]); skillEntry < skillEnd; ++skillEntry)
		{
			std::ofstream logfile("ds_log.txt", std::ios::app);
			if (logfile.is_open()) {
				logfile << "[Skill Debug] Name: " << skillEntry->name
					//<< " | ScreenName: " << skillEntry->screen_name
					//<< " | STR: " << skillEntry->str_influence
					//<< " | DEX: " << skillEntry->dex_influence
					//<< " | INT: " << skillEntry->int_influence
					<< " | XP: " << skillEntry->xp
					<< " | XPNext: " << skillEntry->xp_next_level
					<< " | Base: " << skillEntry->base_level
					<< " | Current: " << skillEntry->current_level
					<< " | Bias: " << skillEntry->level_bias
					<< " | BiasMod: " << skillEntry->level_bias_modifier
					//<< " | Max: " << skillEntry->max_level
					<< std::endl;
			}
		}
	}

	/// Optionally recalculate XP needed for next level using a formula
	FEX void RecalculateXpNextLevel(const char* skill)
	{
		SkillEntry* s = FindSkill(skill);
		if (!s) return;

		// You can replace this formula with your desired progression
		const float growthFactor = 1.25f;
		const float baseXp = 100.0f;
		s->xp_next_level = baseXp * powf(growthFactor, s->base_level);
	}

	/// Adds XP and levels up if threshold reached
	FEX void AddSkillXP(const char* skill, float xpGained)
	{
		SkillEntry* s = FindSkill(skill);
		if (!s) return;

		s->xp += xpGained;
		DebugLogSkillChange(skill, "XP Added", xpGained);

		while (s->xp >= s->xp_next_level)
		{
			s->xp -= s->xp_next_level;
			s->base_level += 1.0f;

			RecalculateXpNextLevel(skill);

			DebugLogSkillChange(skill, "Leveled Up", s->base_level);
		}
	}

private:
	FEX SkillEntry* FindSkill(const char* skill) const
	{
		DWORD* addr = (DWORD*)this;
		SkillEntry* end = (SkillEntry*)(addr[0x07]);
		for (SkillEntry* s = (SkillEntry*)(addr[0x06]); s < end; ++s)
		{
			if (stricmp(skill, s->name) == 0)
				return s;
		}
		return nullptr;
	}

	FEX void DebugLogSkillChange(const char* skill, const char* action, float value)
	{
		std::ofstream logfile("ds_log.txt", std::ios::app);
		if (logfile.is_open()) {
			logfile << "[XP TRACK] Skill: " << skill << " | Action: " << action << " | Value: " << value << std::endl;
		}
	}
};
#endif
