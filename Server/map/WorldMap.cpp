#include "WorldMap.h"
#include <chrono>
#include <ctime>
#include <iomanip>

#include <filesystem>
#include <string>
#include <iostream>

WorldMap g_world;

WorldMap :: ~WorldMap ()
{
	map<string, Region *>::iterator iterator = m_regions.begin();
	while (iterator != m_regions.end())
	{
		delete iterator->second;
		iterator++;
	}
}

void WorldMap::LoadMap(const string& worldname)
{
	m_name = worldname;
	int loadedRegions = 0;

	const string mapDir = "maps/" + worldname + "/regions/";

	if (!fs::exists(mapDir) || !fs::is_directory(mapDir))
	{
		throw runtime_error("region directory does not exist: " + mapDir);
	}

	for (const fs::directory_entry& entry : fs::directory_iterator(mapDir))
	{
		if (!entry.is_directory())
			continue;

		const string regionName = entry.path().filename().string();

		if (m_regions.find(regionName) != m_regions.end())
		{
			throw runtime_error("region already exists in this map: " + regionName);
		}

		try
		{
			Region* region = new Region(worldname, regionName);
			m_regions[regionName] = region;
			m_active.insert(region);
			loadedRegions++;
		}
		catch (const exception& e)
		{
			Log::WriteF(
				Log::Level::ERR,
				"region %s was not loaded because: %s",
				regionName.c_str(),
				e.what()
			);
		}
	}

	Log::Write(
		Log::Level::INFO,
		worldname + " loaded with " + to_string(loadedRegions) + " regions",
		true
	);
}


void WorldMap :: LoadAllMaps()
{
	const string basePath = "maps/";

    try
    {
        for (const auto& entry : fs::directory_iterator(basePath))
        {
            if (entry.is_directory())
            {
                string mapName = entry.path().filename().string();
                try
                {
                    LoadMap(mapName);
                }
                catch (const exception& e)
                {
					Log::Write(Log::Level::ERR, "Failed to load map '" + mapName + "': " + e.what(), true);
                }
            }
        }
    }
    catch (const exception& e)
    {
		Log::Write(Log::Level::ERR, "Error while scanning map directory: " + string(e.what()), true);
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
	string baseDir = "maps\\" + Name();
	string regionFile = baseDir + "\\" + name + ".xml";

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

	// Create and register the Region
	try
	{
		Region *region = new Region(regionFile, name);
		m_regions[name] = region;
		m_active.insert(region);

		Log::Write(Log::Level::INFO, "Created and loaded new region " + name, true);

		return region;
	}
	catch (exception &e)
	{
		Log::WriteF(Log::Level::ERR, "Region %s could not be created because: %s", name.c_str(), e.what());
		return nullptr;
	}
}
