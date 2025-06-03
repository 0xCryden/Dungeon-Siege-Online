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

#ifndef REGION_HPP_
#define REGION_HPP_

	#include "Node.hpp"
	#include "../go/GoDb.hpp"
	
	#include "../vector_3.hpp"
	
	#include <cmath>
	
	class Region
	{
		friend class AIQuery;
		public:
			
			Region (const string & filename, const string & name);
			
			~Region ();
			
			void AddNode(u_int32_t id, float x, float y, float z, u_int8_t rota);

			Node * GetNode(u_int32_t id);

			GopSet & Objects ();
			map<u_int32_t, Node *> GetNodes() { return m_nodes; };
			const string & Name() { return m_name; };
			
		private:

			vector_3 MakeLocalPosition (const SiegePos & position, bool mapMaker);
			void CheckNode (const SiegePos & position);
			
			double GetSiegeDistance (const SiegePos & position, const SiegePos & destination);
			
			string m_name;
			GopSet m_objects;
			map<u_int32_t, Node *> m_nodes;
	};

#endif /* REGION_HPP_ */
