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

#ifndef GOATTACK_HPP_
#define GOATTACK_HPP_

#include "GoComponent.hpp"

#include "../enum/eAttackClass.hpp"
#include "GoMagic.hpp"
	
class GoAttack : public GoComponent
{
	public:

		GoAttack(Go * go);
		GoAttack(Go* go, const GoAttack& other);
		GoAttack(Go * go, xmlNode * node);
		GoAttack(Go* go, const TemplateComponent* tmpl);

		void InheritFrom(const GoAttack& other);
		void Save (xmlNode* attackNode) const;

		int CalcHitType (Go * target, const string & skill, GoMagic * magic = nullptr);
		float CalcDamage (Go * target, const string & skill, int hitType, GoMagic * magic = nullptr);
		uint64_t CalcHitTime ();

		float AttackRange () const;
		float CriticalHitChance () const;

		float DamageMax () const;
		void SetDamageMax(float value) { m_damage_max = value; };

		float DamageMin () const;
		void SetDamageMin(float value) { m_damage_min = value; };

		bool IsInProjectileRange (const Go * go);
		bool IsTwoHanded () const;
		uint64_t ReloadDelay () const;
		eAttackClass AttackClass () const;

		bool IsMelee();
		bool IsRanged();
		bool IsMagic();

	private:

		float m_attack_range;
		float m_critical_hit_chance;
		float m_damage_max;
		float m_damage_min;
		bool m_two_handed;
		uint64_t m_reload_delay;
		eAttackClass m_attack_class;
};

#endif /* GOATTACK_HPP_ */
