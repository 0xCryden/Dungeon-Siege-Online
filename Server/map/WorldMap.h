#pragma once
#include "Region.h"
	
class WorldMap
{
	public:
		~WorldMap ();

		void LoadAllMaps();
		void LoadMap(const string & worldname);
		Region * GetRegion (const string & name);
		set<Region *> & Regions ()
		{
			return m_active;
		} 
		const string & Name() { return m_name; };

	private:
		string m_name;
		set<Region *> m_active;
		map<string, Region *> m_regions;
};
	
extern WorldMap g_world;
