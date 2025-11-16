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

void GoDb::LoadGasToGo()
{
    size_t totalLoaded = 0;
	for (const auto& [instanceName, placement] : placementManager.GetAll())
	{
		const string& templateName = placement.templateName;

		// Skip if already instantiated
		if (m_contentdb.find(templateName) != m_contentdb.end())
			continue;

		if (templateName.empty())
			continue;

		try
		{
			const TemplateData* tmpl = manager.GetTemplate(templateName);
			if (!tmpl)
				throw runtime_error("template not found in TemplateManager");

			// Construct a new Go using your custom constructor
			Go* go = new Go(*tmpl);  // Uses Go(const TemplateData&, const PlacementData&) constructor
			m_contentdb[templateName] = go;

			++totalLoaded;
			//cout << "Instantiated Go from template: " << templateName << endl;
		}
		catch (const exception& e)
		{
			Log::WriteF(Log::Level::ERR, "Failed to instantiate Go from template %s: %s", templateName.c_str(), e.what());
		}
	}
	cout << "[INFO] Finished converting Gas to Gos. Total: " << totalLoaded << endl;
}

void GoDb::LoadGoDbFolder(const string& folderName)
{
    string folderPath = "data\\dynamic\\" + folderName + "\\";
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
	string filePath = "data\\dynamic\\actors\\" + to_string(id) + ".xml";

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
/*
void GoDb::LoadContentDbFolder(const string & folderName)
{
    string folderPath = "data\\static\\" + folderName + "\\";
    string searchPattern = folderPath + "*.xml";

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPattern.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        throw runtime_error("No template XML files found in: " + folderPath);
    }

    do
    {
        string filePath = folderPath + findFileData.cFileName;
        xmlDoc* document = xmlReadFile(filePath.c_str(), NULL, 0);
        if (!document)
        {
            Log::WriteF("Failed to read file: %s", filePath.c_str());
            continue;
        }

        xmlNode* root = xmlDocGetRootElement(document);
        if (!root)
        {
            xmlFreeDoc(document);
            Log::WriteF("Invalid XML in file: %s", filePath.c_str());
            continue;
        }

        for (xmlNode* node = root->children; node != nullptr; node = node->next)
        {
            if (node->type != XML_ELEMENT_NODE || !xmlStrEqual(node->name, BAD_CAST "template"))
                continue;

            string name = xml::ReadAttribute<string>(node, "template_name", "");
            if (name.empty())
                continue;

            if (m_contentdb.find(name) != m_contentdb.end())
            {
                Log::WriteF("Duplicate template name '%s' found in file: %s", name.c_str(), filePath.c_str());
                continue;
            }

            try
            {
                Go* t = new Go(node);
                m_contentdb[name] = t;
                cout << "[CONTENTDB] Loaded template " << name << endl;
            }
            catch (const exception& e)
            {
                Log::WriteF("Template '%s' was not loaded because: %s", name.c_str(), e.what());
            }
        }

        xmlFreeDoc(document);
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}
*/

void GoDb :: LoadContentDb (const string & filename)
{
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

Go * GoDb :: CloneGo (const Go * go)
{
	uint32_t id = NextId();
	
	Go * clone = new Go (id, go);
	m_godb[id] = clone;
	
	return clone;
}

Go * GoDb :: CloneGo (const string & template_name)
{
	map<string, Go *>::iterator iterator = m_contentdb.find (template_name);
	if (iterator != m_contentdb.end())
	{
		uint32_t id = NextId();
		
		Go * go = new Go (id, iterator->second);
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
