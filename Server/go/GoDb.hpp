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

#ifndef GODB_HPP_
#define GODB_HPP_

#include "Go.hpp"
#include "../mysql/MySQL.h"
	
#include <map>

using std::map;

class Account;

class GoDb
{
	public:

		GoDb ();
		~GoDb ();

		void LoadSqlDb();

		void AddGo(Go* go);
		void RemoveGo(Go* go);
		Go* CloneGo(const string& template_name, const string& pcontent_query = "");

		void LoadContentDb();
		void InstantiateMapTemplates();

		void SpawnGo(const string& templateName, const Go* go, const string& pContent);

		Go* FindGoById(uint32_t id);
		Go* FindTemplateByName(const string& template_name);

		void MarkGoForDeletion (uint32_t id);
		void MarkGoAndChildrenForDeletion (uint32_t id);

		uint32_t NextId();

		void CreateChar(Account* account, const string& charName, int charType, int charHead, int charSkin, int charHair, int charShirt, int charPants, std::function<void()> onInserted);
		void DeleteChar(int selectSlot, Account* account, std::function<void()> onInserted);

	private:
		map<uint32_t, Go *> m_godb;
		map<string, Go *> m_contentdb;
};

extern GoDb godb;


#endif /* GODB_HPP_ */
