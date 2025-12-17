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
//#include "../Common.h"

#include "GoDb.hpp"
#include "../Engine.hpp"

#include "../Gas/Gas.hpp"

GoDb godb;

GoDb :: GoDb ()
{
}

GoDb :: ~GoDb ()
{
	{map<uint32_t, Go *>::iterator iterator = m_godb.begin();
	while (iterator != m_godb.end())
	{
		delete iterator->second;
		iterator++;
	}}

	{map<string, Go *>::iterator iterator = m_contentdb.begin();
	while (iterator != m_contentdb.end())
	{
		delete iterator->second;
		iterator++;
	}}
}

/*void GoDb::SpawnGo(const string& templateName, const Go* summonerGo)
{
    Go* tmpl = godb.FindTemplateByName(templateName);
    if (tmpl == NULL) {
        Log::Write(Log::Level::ERR,
            "Unknown template: " + templateName, true);
        return;
    }

    try
    {
        Go* t = new Go(tmpl, *summonerGo->Placement());
        m_godb[t->Goid()] = t;

        string region = t->Placement()->GetRegion();
        if (!region.empty())
        {
            SendWorldMessage(we_entered_world, t, t, region);
            cout << "[GODB] Spawned Go " << NextId() << " using template: " << templateName << " in region: " << region << " at: " << summonerGo->Placement()->Position().X << " | " << summonerGo->Placement()->Position().Y << " | " << summonerGo->Placement()->Position().Z << " in node: " << summonerGo->Placement()->Position().Node << endl;
        }
    }
    catch (exception& e)
    {
        Log::WriteF(Log::Level::ERR, "go %u was not loaded because: %s", NextId(), e.what());
    }
}*/
void GoDb::SpawnGo(const string& templateName, const Go* summonerGo)
{
    const TemplateData* tmpl = manager.GetTemplate(templateName);
    if (!tmpl) {
        Log::Write(Log::Level::ERR,
            "Unknown template: " + templateName, true);
        return;
    }

    try
    {
        Go* t = new Go(*tmpl, *summonerGo->Placement());
        m_godb[NextId()] = t;

        string region = t->Placement()->GetRegion();
        if (!region.empty())
        {
            SendWorldMessage(we_entered_world, t, t, region);
            cout << "[GODB] Spawned Go " << t->Goid() << " using template: " << templateName << " in region: " << region << " at: " << summonerGo->Placement()->Position().X << " | " << summonerGo->Placement()->Position().Y << " | " << summonerGo->Placement()->Position().Z << " in node: " << summonerGo->Placement()->Position().Node << endl;
        }
    }
    catch (exception& e)
    {
        Log::WriteF(Log::Level::ERR, "go %u was not loaded because: %s", NextId(), e.what());
    }
}

void GoDb::GasToGoDb()
{
    int totalLoaded = 0;
    for (auto& [instanceName, placement] : placementManager.GetAll()) {

        const TemplateData* tmpl = manager.GetTemplate(placement.templateName);
        if (!tmpl) {
            Log::Write(Log::Level::ERR,
                "Unknown template: " + placement.templateName, true);
            continue;
        }

        // check if the node exists
        const auto& nodes = g_world.GetRegion(placement.regionName)->GetNodes();
        if (nodes.find(placement.position.Node) == nodes.end()) 
        {
            //std::cout << "Skipping spawn .. region has no nodes" << std::endl;
            continue;
        }

        try
        {
            Go* t = new Go(*tmpl, placement);
            m_godb[NextId()] = t;

            string region = t->Placement()->GetRegion();
            if (!region.empty())
            {
                SendWorldMessage(we_entered_world, t, t, region);
                cout << "[GODB] Spawned Go " << NextId() << " using template: " << placement.templateName << " in region: " << region << " at: " << placement.position.X << " | " << placement.position.Y << " | " << placement.position.Z << " in node: " << placement.position.Node <<  endl;
                totalLoaded++;
            }
        }
        catch (exception& e)
        {
            Log::WriteF(Log::Level::ERR, "go %u was not loaded because: %s", NextId(), e.what());
        }
    }
    cout << "[INFO] Finished converting Gas to Gos. Total: " << totalLoaded << endl;
}

void GoDb::InstantiateMapTemplates()
{
    int totalLoaded = 0;

    // Iterate over SCID map templates instead of placementManager
    auto& maps = manager.GetAllMap();

    for (auto it = maps.begin(); it != maps.end(); )
    {
        // SCID template name == base template name
        TemplateData* baseTpl = manager.GetTemplate(it->second.name);
        if (!baseTpl)
        {
            Log::Write(Log::Level::ERR,
                "SCID template '" + it->second.name +
                "' has no matching base template.", true);
            continue;
        }

        // --------------------------------------------
        // Extract placement data from TemplateData component
        // --------------------------------------------
        const TemplateComponent* placementComp = it->second.GetComponent("placement");
        if (!placementComp)
        {
            Log::Write(Log::Level::ERR,
                "SCID template '" + it->second.name +
                "' has no 'placement' component.", true);
            continue;
        }

        const auto& pc = *placementComp;

        float px = 0, py = 0, pz = 0;
        uint32_t node = 0;

        float ox = 0, oy = 0, oz = 0, ow = 1;

        if (pc.fields.count("p position"))
        {
            const std::string& v = pc.fields.at("p position");
            if (sscanf_s(v.c_str(), "%f,%f,%f,0x%x", &px, &py, &pz, &node) != 4)
                continue; // invalid position format ? skip
        }
        else
            continue; // placement must have a position

        if (pc.fields.count("q orientation"))
        {
            const std::string& v = pc.fields.at("q orientation");
            sscanf_s(v.c_str(), "%f,%f,%f,%f", &ox, &oy, &oz, &ow);
        }
        PlacementData placement;
        // REGION
        placement.regionName = it->second.region;
        placement.position.X = px;
        placement.position.Y = py;
        placement.position.Z = pz;
        placement.position.Node = node;
        // ORIENTATION
        placement.orientation.x = ox;
        placement.orientation.y = oy;
        placement.orientation.z = oz;
        placement.templateName = it->second.name;
        placement.instanceName = it->second.name; // or mapTpl.name + some id

        // --------------------------------------------
        // Check if the node exists in the region
        // --------------------------------------------
        auto region = g_world.GetRegion(placement.regionName);
        if (!region)
        {
            Log::Write(Log::Level::ERR,
                "Region '" + placement.regionName +
                "' does not exist for template '" + it->second.name + "'", true);
            it = maps.erase(it);
            continue;
        }

        const auto& nodes = region->GetNodes();
        if (nodes.find(placement.position.Node) == nodes.end())
        {
            // Region has no such node ? skip spawn
            it = maps.erase(it);
            continue;
        }

        // --------------------------------------------
        // Construct the Go from TemplateData + Placement
        // --------------------------------------------
        try
        {
            Go* go = new Go(it->second, placement);
            uint32_t id = NextId();

            string regionName = go->Placement()->GetRegion();
            if (!regionName.empty())
            {
                m_godb[id] = go;
                SendWorldMessage(we_entered_world, go, go, regionName);

                cout << "[GODB] Spawned Go " << id
                    << " using SCID template: " << it->second.name
                    << " in region: " << regionName
                    << " at: " << placement.position.X
                    << " | " << placement.position.Y
                    << " | " << placement.position.Z
                    << " node: " << placement.position.Node
                    << endl;

                ++totalLoaded;
            }
            else
            {
                delete go;
            }
        }
        catch (const exception& e)
        {
            Log::WriteF(Log::Level::ERR,
                "Go %u failed to load because: %s", NextId(), e.what());
        }

        it = maps.erase(it);
    }

    /*if (TemplateData* tpl = manager.GetMapTemplate("0x032007b5"))
    {
        cout << "[info] template: " << tpl->name << "\n";
        if (!tpl->specializes.empty())
            cout << "  specializes: " << tpl->specializes << "\n";

        for (const auto& [compname, comp] : tpl->components) {
            gas.LogComponent(compname, comp, "  ");
        }
    }*/

    cout << "[INFO] Finished creating Gos from SCID map templates. Total: "
        << totalLoaded << endl;

    size_t templates = manager.GetAll().size();
    cout << "[INFO] templates remaining in cache: "
        << templates << endl;
}

void GoDb::LoadGoDbFolder(const string& folderName)
{
    string folderPath = "data\\" + folderName + "\\";
    string searchPattern = folderPath + "*.xml";

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPattern.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        throw runtime_error("No actor XML files found in: " + folderPath);
    }

    vector<pair<uint32_t, xmlNode*>> withoutInventory;
    vector<pair<uint32_t, xmlNode*>> withInventory;

    do
    {
        string filePath = folderPath + findFileData.cFileName;
        xmlDoc* document = xmlReadFile(filePath.c_str(), NULL, 0);
        if (!document)
        {
            Log::WriteF(Log::Level::ERR, "Failed to read file: %s", filePath.c_str());
            continue;
        }

        xmlNode* root = xmlDocGetRootElement(document);
        if (!root)
        {
            xmlFreeDoc(document);
            Log::WriteF(Log::Level::ERR, "Invalid XML in file: %s", filePath.c_str());
            continue;
        }

        for (xmlNode* node = root->children; node != nullptr; node = node->next)
        {
            if (node->type != XML_ELEMENT_NODE || !xmlStrEqual(node->name, BAD_CAST "go"))
                continue;

            uint32_t id = xml::ReadAttribute<uint32_t>(node, "id", 0);
            if (id == 0)
                continue;

            if (m_godb.find(id) != m_godb.end())
            {
                Log::WriteF(Log::Level::ERR, "Duplicate GO ID %u found in file: %s", id, filePath.c_str());
                continue;
            }

            // Classify GO node by presence of <inventory>
            bool hasInventory = false;
            for (xmlNode* child = node->children; child; child = child->next)
            {
                if (child->type == XML_ELEMENT_NODE && xmlStrEqual(child->name, BAD_CAST "inventory"))
                {
                    hasInventory = true;
                    break;
                }
            }

            auto& targetVec = hasInventory ? withInventory : withoutInventory;
            targetVec.emplace_back(id, xmlCopyNode(node, 1)); // deep copy so doc can be freed

        }

        xmlFreeDoc(document);

    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

    auto processNodes = [&](const vector<pair<uint32_t, xmlNode*>>& nodes)
    {
        for (const auto& [id, node] : nodes)
        {
            try
            {
                Go* t = new Go(node);
                m_godb[id] = t;

                //if (t->HasPlacement())
                if (g_engine.IsPlayer(t) || folderName == "items" || t->Goid() == 5)
                {
                    string region = t->Placement()->GetRegion();
                    if (!region.empty())
                    {
                        SendWorldMessage(we_entered_world, t, t, region);
                        cout << "[GODB] Spawned Go " << id << endl;
                    }
                }

                if (folderName == "items")
                {
                    g_engine.RegisterItem(t);
                }

                if (folderName == "actors")
                {
                    g_engine.RegisterPlayerCharacter(t);
                }
            }
            catch (exception& e)
            {
                Log::WriteF(Log::Level::ERR, "go %u was not loaded because: %s", id, e.what());
            }

            xmlFreeNode(node); // free deep copy after use
        }
    };

    // 1. Load GOs without inventory
    processNodes(withoutInventory);

    // 2. Load GOs with inventory
    processNodes(withInventory);
}

void GoDb :: LoadGoDbSingleChar (uint32_t id)
{
	string filePath = "data\\actors\\" + to_string(id) + ".xml";

    xmlDoc* document = xmlReadFile(filePath.c_str(), NULL, 0);
    if (!document)
    {
        Log::WriteF(Log::Level::ERR, "Failed to open file for GO ID %u: %s", id, filePath.c_str());
        return;
    }

    xmlNode* root = xmlDocGetRootElement(document);
    if (!root)
    {
        xmlFreeDoc(document);
        Log::WriteF(Log::Level::ERR, "Invalid XML structure in file: %s", filePath.c_str());
        return;
    }

    for (xmlNode* node = root->children; node != nullptr; node = node->next)
    {
        if (node->type != XML_ELEMENT_NODE) continue;
        if (!xmlStrEqual(node->name, (const xmlChar*)"go")) continue;

        uint32_t nodeId = xml::ReadAttribute<uint32_t>(node, "id", 0);
        if (nodeId != id) continue;

        try
        {
            Go* t = new Go(node);
            m_godb[id] = t;

            if (g_engine.IsPlayer(t))
            {
                string region = t->Placement()->GetRegion();
                if (!region.empty())
                {
                    SendWorldMessage(we_entered_world, t, t, region);
                    cout << "Loaded go " << id << " from " << filePath << endl;
                }
            }

            g_engine.RegisterPlayerCharacter(t);

            xmlFreeDoc(document);
            return;
        }
        catch (const exception& e)
        {
            Log::WriteF(Log::Level::ERR, "Error constructing GO %u from file %s: %s", id, filePath.c_str(), e.what());
            xmlFreeDoc(document);
            return;
        }
    }

    xmlFreeDoc(document);
    Log::WriteF(Log::Level::ERR, "No matching <go> node with ID %u found in file %s", id, filePath.c_str());
}

void GoDb::LoadContentDb()
{
    //unordered_map<string, TemplateData> templates;
    unordered_set<string> allowed = { "actor", "aspect", "mind" };

    const string rootPath = "data/static/templates";

    size_t totalLoaded = 0;

    for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".gas") {
            const string filePath = entry.path().string();

            unordered_map<string, TemplateData> fileTemplates;
            if (gas.ReadTemplatesFile(filePath, fileTemplates, allowed)) {
                for (auto& [name, tpl] : fileTemplates) {
                    //cout << "Loaded Template: " << name << " from " << filePath << endl;
                    //manager.AddTemplate(move(tpl));
                    map<string, Go*>::iterator iterator = m_contentdb.find(name);
                    if (iterator != m_contentdb.end())
                    {
                        throw runtime_error("template already exists in contentdb");
                    }

                    if (name.empty() != true)
                    {
                        try
                        {
                            Go* t = new Go(tpl);
                            m_contentdb[name] = t;
                            cout << "Loaded template " << name.c_str() << endl;
                        }
                        catch (exception& e)
                        {
                            Log::WriteF(Log::Level::ERR, "template %s was not loaded because : %s", name.c_str(), e.what());
                        }
                    }

                    ++totalLoaded;
                }
            }
        }
    }
    manager.ResolveTemplateInheritance();

    Log::Write(Log::Level::INFO, "[INFO] Finished loading templates. Total loaded: " + to_string(totalLoaded), true);
}

void GoDb :: LoadContentDbOld (const string & filename)
{
    // for loop for all files
    // read all templates

	xmlDoc * document = xmlReadFile (filename.c_str(), NULL, 0);
	if (document == NULL)
	{
		throw runtime_error ("file does not exist");
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
		
		if (xmlStrEqual (node->name, (const xmlChar *) "template") != 0)
		{
			string name = xml::ReadAttribute<string> (node, "template_name", "");
			
			map<string, Go *>::iterator iterator = m_contentdb.find (name);
			if (iterator != m_contentdb.end())
			{
				throw runtime_error ("template already exists in contentdb");
			}
			
			if (name.empty() != true)
			{
				try
				{
					Go * t = new Go (node);
					m_contentdb[name] = t;
					cout << "Loaded template " << name.c_str() << endl;
				}
				catch (exception & e)
				{
					Log::WriteF(Log::Level::ERR, "template %s was not loaded because : %s", name.c_str(), e.what());
				}
			}
		}
	}
	
	xmlFreeDoc (document);
}

Go * GoDb :: FindGoById (uint32_t id)
{
	if (id == 0)
	{
		return NULL;
	}
	
	map<uint32_t, Go *>::iterator iterator = m_godb.find (id);
	return iterator != m_godb.end() ? iterator->second : NULL;
}

Go* GoDb::FindTemplateByName(const string& template_name)
{
    map<string, Go*>::iterator iterator = m_contentdb.find(template_name);
    if (iterator != m_contentdb.end())
    {
        return iterator->second;
    }

    return NULL;
}

Go * GoDb :: CloneGo (const Go * go)
{
	uint32_t id = NextId();
	
	Go * clone = new Go (id, go);
	m_godb[id] = clone;
	
	return clone;
}

/*Go* GoDb::CloneGo(const string& template_name, const GoPlacement& placement)
{
    map<string, Go*>::iterator iterator = m_contentdb.find(template_name);
    if (iterator != m_contentdb.end())
    {
        uint32_t id = NextId();

        Go* go = new Go(id, iterator->second);
        m_godb[id] = go;

        return go;
    }

    return NULL;
}*/

Go* GoDb::CloneGo(const string& template_name)
{
    map<string, Go*>::iterator iterator = m_contentdb.find(template_name);
    if (iterator != m_contentdb.end())
    {
        uint32_t id = NextId();

        Go* go = new Go(id, iterator->second);
        m_godb[id] = go;

        return go;
    }

    return NULL;
}

void GoDb :: MarkGoForDeletion (uint32_t id)
{
	map<uint32_t, Go *>::iterator iterator = m_godb.find (id);
	
	if (iterator != m_godb.end())
	{
		delete iterator->second;
		m_godb.erase (iterator);
	}
}

void GoDb :: MarkGoAndChildrenForDeletion (uint32_t id)
{
	// temporary
	MarkGoForDeletion (id);
}

uint32_t GoDb :: NextId ()
{
	if (m_godb.size() == 0)
	{
		return 1;
	}
	
	if (m_godb.size() == m_godb.rbegin()->first)
	{
		return m_godb.size() + 1;
	}
	
	uint32_t open = 0;
	uint32_t previous = 0;
	map<uint32_t, Go *>::iterator iterator = m_godb.begin();
	
	while (!open)
	{
		if (iterator->first != previous + 1)
		{
			open = previous + 1;
		}
		else
		{
			previous = iterator->first;
		}
		
		++iterator;
	}
	
	return open;
}
