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

#include "WorldMap.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>

#include <filesystem>
#include <string>
#include <iostream>

WorldMap world;

WorldMap :: ~WorldMap ()
{
	map<string, Region *>::iterator iterator = m_regions.begin();
	while (iterator != m_regions.end())
	{
		delete iterator->second;
		iterator++;
	}
}

void WorldMap :: LoadMap (const string & filename, const string & worldname)
{
	m_name = worldname;

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
		
		if (xmlStrEqual (node->name, (const xmlChar *) "region") != 0)
		{
			string name = xml::ReadAttribute<string> (node, "name", "");
			string description = xml::ReadAttribute<string> (node, "description", "");
			string filename = xml::ReadAttribute<string> (node, "filename", "");
			
			map<string, Region *>::iterator iterator = m_regions.find (name);
			if (iterator != m_regions.end())
			{
				throw runtime_error ("region already exists in this map");
			}
			
			if (name.empty() != true)
			{
				try
				{
					Region * region = new Region (filename, name);
					m_regions[name] = region;
					m_active.insert (region);

					auto now = std::chrono::system_clock::now();
					std::time_t now_time = std::chrono::system_clock::to_time_t(now);
					std::cout << "[" << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") << "] "
							  << "Loaded region " << name.c_str() << std::endl;
				}
				catch (exception & e)
				{
					logger.WriteF ("region %s was not loaded because : %s", name.c_str(), e.what());
				}
			}
		}
	}
	
	xmlFreeDoc (document);
}

void WorldMap :: LoadAllMaps()
{
    const std::string basePath = "data//static//map//";
    namespace fs = std::filesystem;

    try
    {
        for (const auto& entry : fs::directory_iterator(basePath))
        {
            if (entry.is_directory())
            {
                std::string mapName = entry.path().filename().string();
                std::string mapFile = basePath + mapName + "//main.xml";

                try
                {
                    LoadMap(mapFile, mapName);
                    cout << "Loaded Map " << mapName << endl;
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Failed to load map '" << mapName << "': " << e.what() << std::endl;
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while scanning map directory: " << e.what() << std::endl;
    }
}

Region * WorldMap :: GetRegion (const string & name)
{
	map<string, Region *>::iterator iterator = m_regions.find (name);
	if (iterator != m_regions.end())
	{
		return iterator->second;
	}
	
	cout << "region does not exist in this map. Creating new region: " << name << endl;

	// Create region XML file path
	string baseDir = "data\\static\\map\\" + Name();
	string regionFile = baseDir + "\\" + name + ".xml";
	string mainFile = baseDir + "\\main.xml";

	// Create the XML file for the region
	ofstream file(regionFile);
	if (!file.is_open())
	{
		cerr << "Failed to create region XML file: " << regionFile << endl;
		return nullptr;
	}
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	file << "<region>\n";
	file << "</region>\n";
	file.close();

	// Load or create main.xml and add region entry
	xmlDoc *doc = xml::LoadFile(mainFile);
	xmlNode *root = nullptr;
	if (!doc)
	{
		// Create new XML doc if not found
		doc = xmlNewDoc(BAD_CAST "1.0");
		root = xmlNewNode(NULL, BAD_CAST "regions");
		xmlDocSetRootElement(doc, root);
	}
	else
	{
		root = xmlDocGetRootElement(doc);
		if (!root)
		{
			root = xmlNewNode(NULL, BAD_CAST "regions");
			xmlDocSetRootElement(doc, root);
		}
	}
	// Construct new <region> entry
	xmlNode *regionNode = xmlNewChild(root, NULL, BAD_CAST "region", NULL);
	xml::SetAttribute(regionNode, "name", name);
	xml::SetAttribute(regionNode, "description", name);
	xml::SetAttribute(regionNode, "filename", "data/static/map/" + Name() + "/" + name + ".xml");

	// Save updated main.xml
	if (!xml::SaveFile(doc, mainFile))
	{
		std::cerr << "Failed to update main.xml with region node for " << name << std::endl;
		xmlFreeDoc(doc);
		return nullptr;
	}

	xmlFreeDoc(doc);

	// Create and register the Region
	try
	{
		Region *region = new Region(regionFile, name);
		m_regions[name] = region;
		m_active.insert(region);

		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::cout << "[" << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") << "] "
				  << "Created and loaded new region " << name << std::endl;

		return region;
	}
	catch (exception &e)
	{
		logger.WriteF("Region %s could not be created because: %s", name.c_str(), e.what());
		return nullptr;
	}
}
