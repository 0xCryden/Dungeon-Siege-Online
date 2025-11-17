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
	m_region = go->Placement()->m_region;
	m_position.Node = go->Placement()->m_position.Node;
	m_position.X = go->Placement()->m_position.X;
	m_position.Y = go->Placement()->m_position.Y;
	m_position.Z = go->Placement()->m_position.Z;
}

GoPlacement :: GoPlacement (Go * go, xmlNode * node) : GoComponent (go)
{
	if (node != NULL)
	{
		m_region = xml::ReadAttribute<string> (node, "region", "");
		
		xmlNode * current = NULL;
		for (current = node->children; current != NULL; current = current->next)
		{
			if (current->type != XML_ELEMENT_NODE) continue;
			
			if (xmlStrEqual (current->name, (const xmlChar *) "position") != 0)
			{
				m_position.Node = xml::ReadAttribute<uint32_t> (current, "node", 0);
				m_position.X = xml::ReadAttribute<float> (current, "x", 0.0f);
				m_position.Y = xml::ReadAttribute<float> (current, "y", 0.0f);
				m_position.Z = xml::ReadAttribute<float> (current, "z", 0.0f);

				// Logging the placement info
				/*std::cout << "[GoPlacement] Placed object with GO ID=" << go->Goid()
				          << " in region=\"" << m_region << "\" at node=" << m_position.Node
				          << " (" << std::fixed
				          << m_position.X << ", " << m_position.Y << ", " << m_position.Z << ")"
				          << std::endl;*/
			}
		}
	}
}

GoPlacement::GoPlacement(Go* go, const GoPlacement& placement) : GoComponent(go)
{
	m_region = placement.m_region;
	m_position.Node = placement.m_position.Node;
	m_position.X = placement.m_position.X;
	m_position.Y = placement.m_position.Y;
	m_position.Z = placement.m_position.Z;
}

GoPlacement::GoPlacement(Go* go, const PlacementData& data) : GoComponent(go)
{
	m_region = data.regionName;
	m_position.Node = data.position.Node;
	m_position.X = data.position.X;
	m_position.Y = data.position.Y;
	m_position.Z = data.position.Z;

	// TODO implement m_rotation into GoPlacement class
	//m_orientation = data.orientation;
}

void GoPlacement :: Save(xmlNode* placementNode) const
{
	xml::SetAttribute(placementNode, "region", m_region);

    // Search for an existing position node.
    xmlNode* positionNode = nullptr;
    for (xmlNode* child = placementNode->children; child; child = child->next)
    {
        if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, (const xmlChar*)"position"))
        {
            positionNode = child;
            break;
        }
    }

    // If the position node doesn't exist, create a new one.
    if (!positionNode)
    {
        positionNode = xmlNewChild(placementNode, nullptr, BAD_CAST "position", nullptr);
    }

    // Update the position node attributes with the current values.
    xml::SetAttribute(positionNode, "node", m_position.Node);
    xml::SetAttribute(positionNode, "x", m_position.X);
    xml::SetAttribute(positionNode, "y", m_position.Y);
    xml::SetAttribute(positionNode, "z", m_position.Z);
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
