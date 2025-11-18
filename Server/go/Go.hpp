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

#ifndef GO_HPP_
#define GO_HPP_

#include "GopSet.hpp"
	
#include "GoActor.hpp"
#include "GoAspect.hpp"
#include "GoAttack.hpp"
#include "GoBody.hpp"
#include "GoCommon.hpp"
#include "GoDefend.hpp"
#include "GoGui.h"
#include "GoInventory.hpp"
#include "GoMagic.hpp"
#include "GoMind.h"
#include "GoPlacement.hpp"
#include "GoScriptComponent.hpp"

#include "../helper/vector_3.h"
#include "../Gas/Gas.hpp"
	
/*
	* GoActor
	* GoAspect
	* GoAttack
	* GoBody
	* GoCommon
	* GoDefend
	* GoGui
	* GoInventory
	* GoMagic
	* GoMind
	* GoParty
	* GoPlacement
	*/
	
struct sqlResult; // temp until sql implemented
	
class Go
{
	public:
			
		Go (xmlNode * node); // ONLY USED FOR CREATING NEW TEMPLATES
		void LoadFromXml(xmlNode* node);
		Go (sqlResult * query); // only used for loading a 'save game'
		Go (uint32_t id, const Go * go); // used for creating a new go from either [an existing go, or a template]
		Go(const TemplateData& tmpl, const PlacementData& placement);
		Go(const TemplateData& tmpl, const GoPlacement& placement);
		~Go ();

		GoActor * Actor () const;

		eEquipSlot IntendedSlot ();
		eInventoryLocation IntendedLoc ();
		void HandleCommand (const string& command);
		void SaveToXml(const std::string& folderName);
		string GetTitle();
		void AddChild (Go * child);
		GoAspect * Aspect () const;
		GoAttack * Attack () const;
		GoBody * Body () const;
		const GopSet & Children () const;
		void ClearParent ();
		GoCommon * Common () const;
		// GoConversation * Conversation () const;
		GoDefend * Defend () const;
		GoGui* Gui() const;
		const GopSet & Frustum ();
		GoScriptComponent * GetComponent (const string & component) const;
		uint32_t Goid () const;
		// GoGui * Gui () const;
		bool HasActor () const;
		bool HasAspect () const;
		bool HasAttack () const;
		bool HasBody () const;
		bool HasChild (Go * child) const;
		bool HasCommon () const;
		bool HasComponent (const string & component) const;
		bool HasConversation () const; // code me
		bool HasDefend () const;
		bool HasGui () const; // code me
		bool HasInventory () const;
		bool HasMagic () const; // has magic
		bool HasMind () const;
		// bool HasParty () const; 
		bool HasPlacement () const;
		GoInventory * Inventory () const;
		bool IsActor () const;
		// bool IsAnyHumanPartyMember () const;
		bool IsArmor () const;
		bool IsBreakable () const; // code me
		bool IsContainer () const; // code me
		bool IsEquipped () const;
		bool IsGhost () const; // code me!
		bool IsInsideInventory () const;
		bool IsItem () const;
		bool IsMeleeWeapon () const;
		bool IsRangedWeapon () const;
		bool IsSpell () const; // code me
		bool IsSpellBook () const; // code me
		bool IsTeamMember (const Go * go) const; // code me
		bool IsWeapon () const;
		eLifeState LifeState () const;
		GoMagic * Magic () const;
		GoMind * Mind () const;
		Go * Parent () const;
		GoPlacement * Placement () const;
		void RemoveAllChildren ();
		void RemoveChild (Go * child);
		void Send (const WorldMessage & message);
		void SetParent (Go * parent);
		// void SetPlayer( const PlayerId * ) ??????
		string TemplateName () const;
			
			
		// depreciate please
		void AddComponent (GoScriptComponent * component);
		void RemoveComponent (const string & component);
			
		void SetLoc(eInventoryLocation loc) {
			//cout << "Set loc id " << Goid() << " loc: " << (eInventoryLocation)loc << endl;
			m_inventoryLocation = loc;
			/*Parent()->Inventory()->SetBagLoc(this, loc);*/ }

		eInventoryLocation GetLoc() {
			//cout << "Get loc id " << Goid() << " loc " << (eInventoryLocation)m_inventoryLocation << endl;
			return m_inventoryLocation; }

		void SetOwner(int ownerId) { m_inventoryOwnerId = ownerId; }
		int GetOwner() { return m_inventoryOwnerId; }

		void SetLastLocal(vector_3 lastLoc) { lastLocal = lastLoc; }
		vector_3 GetLastLocal() { return lastLocal; }

		void SetLastPos(SiegePos lastPosi) { lastPos = lastPosi; }
		SiegePos GetLastPos() { return lastPos; }

		void SetWaitForNodeInfo(bool is) { waitForNodeInfo = is; }
		bool WaitForNodeInfo() { return waitForNodeInfo; }

		void SetLastRota(uint8_t lastRot) { lastRota = lastRot; }
		uint8_t GetLastRota() { return lastRota; }

		void CalculateStatus();

		double GetDistanceTo(Go * target);

		int Admin() { return m_admin; }
		void SetAdmin(int level) { m_admin = (uint8_t)level; }
		vector<string> Conversations() { return m_conversations; }

	private:
			
		uint32_t m_goid;
		string m_template_name;
			
		GopSet m_objects;
			
		Go * m_parent;
		GopSet m_children;
			
		GoActor * m_actor;
		GoAspect * m_aspect;
		GoAttack * m_attack;
		GoBody * m_body;
		GoCommon * m_common;
		GoDefend* m_defend;
		GoGui* m_gui;
		GoInventory * m_inventory;
		GoMagic * m_magic;
		GoMind * m_mind;
		GoPlacement * m_placement;
			
		eInventoryLocation m_inventoryLocation = il_main;
		int m_inventoryOwnerId = 0;
		SiegePos lastPos;
		vector_3 lastLocal;
		uint8_t lastRota = 0;
		bool waitForNodeInfo = false;
		uint8_t m_admin = 0;

		map<string, GoScriptComponent*> m_scripts;

		vector<string> m_conversations;
};

#endif /* GO_HPP_ */
