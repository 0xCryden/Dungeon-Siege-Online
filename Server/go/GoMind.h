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
};

#endif /* GOMIND_H_ */
