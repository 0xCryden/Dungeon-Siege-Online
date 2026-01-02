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
#include "GoConversation.h"

#include "../helper/vector_3.h"
#include "../Gas/Gas.hpp"
#include "../server/Account.hpp"

#include "../mysql/MySQL.h"

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
	
using QueryResult = std::vector<std::map<std::string, std::string>>;

class Account;
//class MySQL;

class Go
{
	public:
		Go(const TemplateData& tmpl, uint32_t goid = 0, const string& pcontent = "");	// for pContent (if goid == 0) & Instantiating new templates
		Go(const Go* go, uint32_t goid, const string& pcontent_query);			// (CloneGo) used for creating a new go from either [an existing go, or a template]
		~Go ();

		void Save(MySQL& db);

		uint32_t Goid() const;
		uint32_t Scid() const;
		string TemplateName() const;
		string pContentQuery() const;

		GoActor * Actor () const;
		GoAspect* Aspect() const;
		GoAttack* Attack() const;
		GoCommon* Common() const;
		GoDefend* Defend() const;
		GoGui* Gui() const;
		GoBody* Body() const;
		GoInventory* Inventory() const;
		GoMagic* Magic() const;
		GoMind* Mind() const;
		GoPlacement* Placement() const;
		// TODO make GoConveration
		GoConversation* Conversation() const;

		const GopSet& Children() const;
		GoScriptComponent* GetComponent(const string& component) const;
		Account* GetAccount() const { return m_account; }

		void CopyPlacement(GoPlacement* placement);

		void SetAccount(Account* acc) { m_account = acc; }
		void AddChild(Go* child);
		void AddComponent(GoScriptComponent* component);

		bool HasActor() const;
		bool HasAspect() const;
		bool HasAttack() const;
		bool HasCommon() const;
		bool HasDefend() const;
		bool HasGui() const;
		bool HasBody() const;
		bool HasInventory() const;
		bool HasMagic() const;
		bool HasMind() const;
		bool HasPlacement() const;
		bool HasConversation() const;

		bool HasChild(Go* child) const;
		bool HasComponent(const string& component) const;

		bool IsActor() const;
		bool IsItem() const;
		bool IsArmor() const;
		bool IsWeapon() const;
		bool IsMeleeWeapon() const;
		bool IsRangedWeapon() const;
		bool IsSpell() const; // code me
		bool IsSpellBook() const; // code me
		bool IsEquipped() const;
		bool IsBreakable() const; // code me
		bool IsContainer() const; // code me
		bool IsGhost() const; // code me!

		void LoadFromDatabase(MySQL& db, std::function<void(Go*)> onLoaded);
		void LoadPlacement(MySQL& db, std::function<void()> done);
		void LoadActor(MySQL& db, std::function<void()> done);
		void LoadAspect(MySQL& db, std::function<void()> done);
		void LoadCommon(MySQL& db, std::function<void()> done);
		void LoadInventory(MySQL& db, std::function<void()> done);
		void LoadAttack(MySQL& db, std::function<void()> done);
		void LoadDefend(MySQL& db, std::function<void()> done);
		void LoadMind(MySQL& db, std::function<void()> done);
		void LoadBody(MySQL& db, std::function<void()> done);
		void LoadGui(MySQL& db, std::function<void()> done);
		void LoadMagic(MySQL& db, std::function<void()> done);

		// TODO move
		eInventoryLocation IntendedLoc () const;
		void SetLoc(eInventoryLocation loc) { m_inventoryLocation = loc; }
		eInventoryLocation GetLoc() { return m_inventoryLocation; }
		// to placement component
		double GetDistanceTo(Go* target) const;
		void SetLastLocal(vector_3 lastLoc) { lastLocal = lastLoc; }
		vector_3 GetLastLocal() { return lastLocal; }
		void SetLastPos(SiegePos lastPosi) { lastPos = lastPosi; }
		SiegePos GetLastPos() { return lastPos; }
		void SetWaitForNodeInfo(bool is) { waitForNodeInfo = is; }
		bool WaitForNodeInfo() { return waitForNodeInfo; }
		void SetLastRota(uint8_t lastRot) { lastRota = lastRot; }
		uint8_t GetLastRota() { return lastRota; }


		void HandleCommand(const string& command);
		void ClearParent ();
		const GopSet & Frustum ();
		bool IsInsideInventory () const;
		bool IsTeamMember (const Go * go) const; // code me
		Go * Parent () const;
		void RemoveAllChildren ();
		void RemoveChild (Go * child);
		void Send (const WorldMessage & message);
		void SetParent (Go * parent);
		// void SetPlayer( const PlayerId * ) ??????
		// bool HasParty () const; 
		// bool IsAnyHumanPartyMember () const;

		// depreciate please
		void RemoveComponent (const string & component);

		void CalculateStatus();

	private:

		uint32_t m_goid;
		uint32_t m_scid;
		string m_template_name;
		string m_pcontent_query;
		string m_specializes;
			
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
		GoConversation* m_conversation;

		Account* m_account;
			
		eInventoryLocation m_inventoryLocation = il_main;
		SiegePos lastPos;
		vector_3 lastLocal;
		uint8_t lastRota = 0;
		bool waitForNodeInfo = false;

		map<string, GoScriptComponent*> m_scripts;
};

#endif /* GO_HPP_ */
