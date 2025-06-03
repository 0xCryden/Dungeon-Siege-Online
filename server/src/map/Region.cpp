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

#include "Region.hpp"
#include <fstream>
#include <iomanip>
#include "../net/packets.hpp"
#include "../net/Connection.hpp"
#include "../Go/Go.hpp"

#include "../map/WorldMap.hpp"

Region :: Region (const string & filename, const string & name)
{
	m_name = name;

	xmlDoc * document = xmlReadFile (filename.c_str(), NULL, 0);
	if (document == NULL)
	{
		world.GetRegion(name);
		//throw runtime_error ("file does not exist");
	}
	
	xmlNode * root = xmlDocGetRootElement (document);
	if (root == NULL)
	{
		xmlFree (document);
		throw runtime_error ("file is not valid xml");
	}
	
	xmlNode * node = NULL;
	for (node = root->children; node != NULL; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE) continue;
		
		if (xmlStrEqual (node->name, (const xmlChar *) "node") != 0)
		{
			u_int32_t id = xml::ReadAttribute<u_int32_t> (node, "id", 0);
			float x = xml::ReadAttribute<float> (node, "x", 0.0f);
			float y = xml::ReadAttribute<float> (node, "y", 0.f);
			float z = xml::ReadAttribute<float> (node, "z", 0.0f);
			u_int8_t rotation = xml::ReadAttribute<int> (node, "rotation", 0);
			
			map<u_int32_t, Node *>::iterator iterator = m_nodes.find (id);
			if (iterator != m_nodes.end())
			{
				cout << "Error loading node: " << id << endl;
				//throw runtime_error ("node already exists in this region");
			}

			if (id != 0)
			{
				Node * node = new Node (id, x, y, z, rotation);
				m_nodes[id] = node;
			}
		}
	}
	
	xmlFreeDoc (document);
}

void Region :: AddNode (u_int32_t id, float x, float y, float z, u_int8_t rota)
{
	// Load node into server dynamically
	if (id != 0)
	{
		Node * node = new Node (id, x, y, z, rota);
		m_nodes[id] = node;
	}
	// Add new node to .xml file
	// Read existing content and remove </region>
	const string filename = "data\\static\\map\\" + world.Name() + "\\" + Name() + ".xml";
	ifstream inFile(filename);
	stringstream buffer;
	string line;
	bool regionTagFound = false;

	while (getline(inFile, line))
	{
		if (line.find("</region>") != string::npos)
		{
			regionTagFound = true;
			continue; // Skip writing </region> for now
		}
		buffer << line << "\n";
	}
	inFile.close();

	// Append new node
	buffer << fixed << setprecision(2);
	buffer << "\t<node id=\"" << id << "\" x=\"" << x << "\" y=\"" << y << "\" z=\"" << z << "\" rotation=\"" << int(rota) << "\" />\n";

	// Append </region> back
	if (regionTagFound)
		buffer << "</region>\n";

	// Write back to file
	ofstream outFile(filename, ios::trunc);
	outFile << buffer.str();
	outFile.close();
	cout << "Creating node ID " << id << " at " << x << "/" << y << "/" << z << " orientation: " << int(rota) << endl;
}

Region :: ~Region ()
{
	map<u_int32_t, Node *>::iterator iterator = m_nodes.begin();
	while (iterator != m_nodes.end())
	{
		delete iterator->second;
	}
}

GopSet & Region :: Objects ()
{
	return m_objects;
}

vector_3 Region :: MakeLocalPosition (const SiegePos & position, bool mapMaker)
{
	map<u_int32_t, Node *>::iterator iterator = m_nodes.find (position.Node);
	if (iterator != m_nodes.end())
	{
		Node * node = iterator->second;
		vector_3 local;
		local.x = node->X();
		local.y = node->Y();
		local.z = node->Z();

		int rotation = node->Rotation();
		switch (rotation) // 0 - unknown, 1 - north, 2 - east, 3 - south, 4 - west
		{
			case 1: local.x += position.X; local.z += position.Z; break;
			case 2: local.x -= position.Z; local.z += position.X; break;
			case 3: local.x -= position.X; local.z -= position.Z; break;
			case 4: local.x += position.Z; local.z -= position.X; break;
			default: throw runtime_error("invalid node");
		}
		return local;
	}
	throw range_error ("node does not exist in this region");
}

double Region :: GetSiegeDistance (const SiegePos & position, const SiegePos & destination)
{
	vector_3 p;
	vector_3 d;
	
	try
	{
		p = MakeLocalPosition (position, true);
		//cout << "Absolute Position: " << p.x << "/" << p.y << "/" << p.z << endl;
		d = MakeLocalPosition (destination, false);
	}
	catch (exception & e)
	{
		logger.WriteF ("caught an exception while calling GetSiegeDistance : %s", e.what());
		return 0.0;
	}
	 
	double x = p.x - d.x;
	double y = p.y - d.y;
	double z = p.z - d.z;
	
	return sqrt (x * x + y * y + z * z);
}

Node * Region :: GetNode (u_int32_t id)
{
	map<u_int32_t, Node *>::iterator iterator = m_nodes.find (id);
	if (iterator != m_nodes.end())
	{
		Node * node = iterator->second;

		return node;
	}
	else
	{
		cout << "GetNode node ID not found" << endl;
		return nullptr;
	}
}
