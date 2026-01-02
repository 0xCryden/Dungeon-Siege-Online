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
#include "../Common.h"

#include "Go.hpp"
#include "GoPlacement.hpp"
#include "../map/WorldMap.h"
#include "../Engine.hpp"

GoPlacement :: GoPlacement (Go * go) : GoComponent (go)
{
}

GoPlacement::GoPlacement(Go* newGo, const GoPlacement& other) : GoComponent(newGo) // attach to new Go
{
	m_region = other.m_region;

	m_position.Node = other.m_position.Node;
	m_position.X = other.m_position.X;
	m_position.Y = other.m_position.Y;
	m_position.Z = other.m_position.Z;
}

GoPlacement::GoPlacement(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
	if (tmplComp == nullptr)
		return;

	float px = 0, py = 0, pz = 0;
	uint32_t node = 0;

	if (tmplComp->fields.count("p position"))
	{
		const std::string& v = tmplComp->fields.at("p position");
		if (sscanf_s(v.c_str(), "%f,%f,%f,0x%x", &px, &py, &pz, &node) != 4)
			return; // invalid position format ? skip
	}
	else
		return; // placement must have a position

	/*
	float ox = 0, oy = 0, oz = 0, ow = 1;
	if (tmplComp->fields.count("q orientation"))
	{
		const std::string& v = tmplComp->fields.at("q orientation");
		sscanf_s(v.c_str(), "%f,%f,%f,%f", &ox, &oy, &oz, &ow);
	}*/

	m_position.X = px;
	m_position.Y = py;
	m_position.Z = pz;
	m_position.Node = node;
	m_region = "town_center";
}

GoPlacement::GoPlacement(Go* go, const std::map<std::string, std::string>& r) : GoComponent(go)
{
	m_region = r.at("region");
	m_position.Node = std::stoul(r.at("node_id"));
	m_position.X = std::stof(r.at("pos_x"));
	m_position.Y = std::stof(r.at("pos_y"));
	m_position.Z = std::stof(r.at("pos_z"));

	// TODO implement m_rotation into GoPlacement class
	//m_orientation = data.orientation;
}

void GoPlacement::Save(MySQL& db)
{
	std::string q =
		"INSERT INTO t_go_placement (go_id, region, node_id, pos_x, pos_y, pos_z) VALUES (" +
		std::to_string(GetGo()->Goid()) + ", '" +
		m_region + "', " +
		std::to_string(m_position.Node) + ", " +
		std::to_string(m_position.X) + ", " +
		std::to_string(m_position.Y) + ", " +
		std::to_string(m_position.Z) + ") "
		"ON DUPLICATE KEY UPDATE "
		"region=VALUES(region), "
		"node_id=VALUES(node_id), "
		"pos_x=VALUES(pos_x), "
		"pos_y=VALUES(pos_y), "
		"pos_z=VALUES(pos_z)";

	db.AsyncQuery(q, [](const auto&) {});
}

bool GoPlacement :: IsDirty () const
{
	return m_dirty;
}

void GoPlacement :: MarkAsClean ()
{
	//cout << "marked GO as clean " << endl;
	m_dirty = false;
}

string GoPlacement :: GetRegion () const
{
	return m_region;
}

void GoPlacement :: SetRegion (const string & region)
{
	m_region = region;
}

SiegePos GoPlacement :: Position () const
{
	return m_position;
}

void GoPlacement :: SetPosition (const SiegePos & position)
{
	//cout << "marking as dirty" << endl;
	m_dirty = true;
	m_position = position;

	Region * region = g_world.GetRegion (GetRegion());
	Node * node = region->GetNode(position.Node);

	if (node != nullptr)
	{
		vector_3 local;
		local.x = node->X();
		local.y = node->Y();
		local.z = node->Z();

		int rotation = node->Rotation();
		switch (rotation) // 0 - unknow, 1 - north, 2 - east, 3 - south, 4 - west
		{
			case 1: local.x += position.X; local.z += position.Z; break;
			case 2: local.x -= position.Z; local.z += position.X; break;
			case 3: local.x -= position.X; local.z -= position.Z; break;
			case 4: local.x += position.Z; local.z -= position.X; break;
			default: throw runtime_error("invalid node");
		}

		lastLocal.x = node->X();
		lastLocal.y = node->Y();
		lastLocal.z = node->Z();
		//save new SiegePos (X/Y/Z inside a node and NodeID)
		lastSiegePos = position;
		//save new Rotation
		lastRotation = node->Rotation();

		double absoluteDistanceMoved = region->GetDistance(local, WorldPosition());

		/*cout << "Distance traveled: " << absoluteDistanceMoved << endl;
		Go* mapCenterGo = godb.FindGoById(5);
		if (mapCenterGo != NULL)
		{
			cout << "Distance to 0/0/0: " << region->GetSiegeDistance(mapCenterGo->Placement()->Position(), GetGo()->Placement()->Position()) << endl;
		}*/

		if (region->GetDistance(local, WorldPosition()) > 10.0)
		{
			cout << "###### !!! Warning player position jumped distance > 10, wrong node locations received?" << endl;
		}

		SetWorldPosition(local);
	}
	else
	{
		Go* playerChar = GetGo();
		if ((g_engine.IsPlayer(playerChar) == true) && (playerChar->WaitForNodeInfo() == false))
		{
			playerChar->SetLastPos(lastSiegePos);
			playerChar->SetLastLocal(lastLocal);
			playerChar->SetLastRota(lastRotation);
			playerChar->Send(WorldMessage (we_unknown_node, playerChar, playerChar, ""));
			playerChar->SetWaitForNodeInfo(true);
		}
		else
		{
			cout << "Still waiting for node info.." << endl;
		}

	}
}

vector_3 GoPlacement :: WorldPosition () const
{
	return m_worldPosition;
}

void GoPlacement :: SetWorldPosition (vector_3 worldPosition)
{
	m_worldPosition = worldPosition;
}

void GoPlacement :: SetWorldPosition (float x, float y, float z)
{
	m_worldPosition.x = x;
	m_worldPosition.y = y;
	m_worldPosition.z = z;
}
