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
	
#include <map>

using std::map;

class GoDb
{
	public:

		GoDb ();

		~GoDb ();

		void LoadGasToGo();

		void LoadGoDbFolder (const string & folderName);
		void LoadGoDbSingleChar (uint32_t id);

		void LoadContentDb (const string & filename);
		void GasToGoDb();
		void SpawnGo(const string& templateName, const Go* go);
		//void LoadContentDbFolder (const string & folderName);
		//void LoadContentDbSingle (const string & filename);

		Go * FindGoById (uint32_t id);

		Go * CloneGo (const Go * go);
		Go * CloneGo (const string & template_name);
		//Go * GetTemplate (const string & templateName);
		void MarkGoForDeletion (uint32_t id);
		void MarkGoAndChildrenForDeletion (uint32_t id);

		uint32_t NextId();

	private:


		map<uint32_t, Go *> m_godb;
		map<string, Go *> m_contentdb;
};

extern GoDb godb;


#endif /* GODB_HPP_ */
