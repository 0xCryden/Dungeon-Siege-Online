#ifndef GOMIND_H_
#define GOMIND_H_

#include "GoComponent.hpp"
#include "../enum/eJobAbstractType.hpp"
#include "../enum/eEquipSlot.hpp"
#include "../enum/eInventoryLocation.hpp"
#include "../helper/CurrentTime.hpp"
#include "../helper/SiegePos.h"
//#include "../utils/AIQuery.hpp"

class GoMind : public GoComponent
{
	public:
			
		GoMind (Go * go);
		GoMind (Go * go, xmlNode * node);
		GoMind(Go* go, const TemplateComponent* tmplComp);

		int64_t TimeElapsedSinceLastMeleeAttack () const;
		int64_t TimeElapsedSinceLastRangedAttack () const;
		int64_t TimeElapsedSinceLastSpellCast () const;
			
		void Stop ();
		void Move (const SiegePos & position);
		void Get (Go * item);
		void Drop (Go * item);
		void Equip (eEquipSlot slot, Go * item);
		void Unequip (eEquipSlot slot);
		void AttackMelee (Go * target);
		void AttackRanged (Go * target);
		void Cast (Go * target);
			
		void ContinueLastAction ();
			
		eJobAbstractType ActionJat () const;
		SiegePos ActionPosition () const;
		float ActionDistance () const;
		Go * ActionObject () const;
		eEquipSlot ActionSlot () const;
		eInventoryLocation ActionLocation () const;
			
	private:
			
		int64_t m_melee;
		int64_t m_ranged;
		int64_t m_casting;
			
		eJobAbstractType m_jat;
		SiegePos m_position;
		float m_distance;
		Go * m_object;
		eEquipSlot m_slot;
		eEquipSlot m_oldslot;
		eEquipSlot m_oldslot2;
		eInventoryLocation m_location;

		bool m_actor_auto_defends_others;
		bool m_actor_auto_heals_others_life;
		bool m_actor_auto_switches_to_karate;
		bool m_actor_auto_switches_to_melee;
		//eWeaponPreference actor_weapon_preference;

		float m_actor_life_ratio_high_threshold;
		float m_actor_life_ratio_low_threshold;
		float m_actor_mana_ratio_high_threshold;
		float m_actor_mana_ratio_low_threshold;

		bool m_actor_may_attack;
		bool m_actor_may_be_attacked;

		string m_jat_brain;
		string m_jat_listen;
		float m_melee_engage_range;

		//eMovementOrders movement_orders = mo_limited;
		bool m_on_enemy_entered_icz_switch_to_melee;
		bool m_on_engaged_lost_consciousness_abort_attack;

		float m_ranged_engage_range;
		float m_sensor_scan_period;
		float m_sight_range;
};

#endif /* GOMIND_H_ */
