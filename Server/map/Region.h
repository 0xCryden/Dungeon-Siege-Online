#pragma once
#include "Node.h"
#include "../go/GoDb.hpp"
#include "../helper/vector_3.h"
#include <cmath>
	
class Region
{
	friend class AIQuery;
	public:			
		Region (const string & filename, const string & name);
		~Region ();
			
		void AddNode(uint32_t id, float x, float y, float z, uint8_t rota);
		Node * GetNode(uint32_t id);
		GopSet & Objects ();
		map<uint32_t, Node *> GetNodes() { return m_nodes; };
		const string & Name() { return m_name; };
		double GetSiegeDistance (const SiegePos & position, const SiegePos & destination);
			
	private:
		vector_3 MakeLocalPosition (const SiegePos & position, bool mapMaker);
		//void CheckNode (const SiegePos & position);
		//double GetSiegeDistance (const SiegePos & position, const SiegePos & destination);
			
		string m_name;
		GopSet m_objects;
		map<uint32_t, Node *> m_nodes;
};
