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

#ifndef GOACTOR_HPP_
#define GOACTOR_HPP_

#include "../Common.h"

#include "Skill.hpp"
#include "../enum/eActorAlignment.hpp"
#include "GoComponent.hpp"

#include "../Gas/Gas.hpp"

class MySQL;
	
class GoActor : public GoComponent
{
public:
			
	GoActor (Go * go);
	GoActor (Go* go, const TemplateComponent* tmpl);
	GoActor(Go* go, const std::map<std::string, std::string>& r);
	~GoActor ();

	void Save(MySQL& db);

	eActorAlignment Alignment () const;
	bool CanLevelUp() const;
	void SetCanLevelUp(bool can);
	map<string, Skill*> Skills();
	string GetTitle() const;

	float GetLevelFromXP(double xp);
	double GetXPFromLevel(float level);
	double GetMaxExpGainForLevel(float level);
	void ChangeSkillLevel (const string & skill, float delta);
	bool HasSkill (const string & skill) const;
	float GetSkillLevel (const string & skill) const;
	double GetSkillExp (const string & skill) const;
	float HighestSkillLevel () const;
	void SetAlignment (eActorAlignment alignment);
	void SetSkillLevel (const string & skill, float value);
	void SetSkillExp (const string & skill, double value);

	void AddSkillExp (const string & skill, double value);

private:
			
	eActorAlignment m_alignment;
	map<string, Skill *> m_skills;
	bool m_can_level_up;
};

#endif /* GOACTOR_HPP_ */
