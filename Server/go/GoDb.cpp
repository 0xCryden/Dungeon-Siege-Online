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

#include "../server/Account.hpp"

std::vector<std::string> modelStrings = {
        "humanboy",
        "humangirl",
        "dso_dwarf",
        "dso_skeleton",
        "dso_utraean_boy",
        "dso_utraean_girl",
        "dso_halfgiant"
};

std::vector<std::vector<std::string>> headStrings = {
    // humanboy (portal_farmboy)
    {
        "default", "m_c_gah_fb_hlmt_head-02", "m_c_gah_fb_hlmt_head-03", "m_c_gah_fb_hlmt_head-04", "m_c_gah_fb_hlmt_head-05"
    },
    // humangirl (portal_farmgirl)
    {
        "default",  "m_c_gah_fg_hlmt_head-02", "m_c_gah_fg_hlmt_head-03", "m_c_gah_fg_hlmt_head-04", "m_c_gah_fg_hlmt_head-05"
    },
    // dwarf (portal_dwarf)
    {
        "default"
    },
    // skeleton (portal_skeleton)
    {
        "default"
    },
    // utraean_boy (portal_utraean_boy)
    {
        "default", "m_c_gah_fb_hlmt_head-02", "m_c_gah_fb_hlmt_head-03", "m_c_gah_fb_hlmt_head-04", "m_c_gah_fb_hlmt_head-05"
    },
    // utraean_girl (portal_utraean_girl)
    {
        "default", "m_c_gah_fg_hlmt_head-02", "m_c_gah_fg_hlmt_head-03", "m_c_gah_fg_hlmt_head-04", "m_c_gah_fg_hlmt_head-05"
    },
    // halfgiant (portal_halfgiant)
    {
        "default"
    }
};

std::vector<std::vector<std::string>> skinStrings = {
    // humanboy
    {
        "b_c_gah_fb_skin_001", "b_c_gah_fb_skin_002", "b_c_gah_fb_skin_003", "b_c_gah_fb_skin_004",
        "b_c_gah_fb_skin_005", "b_c_gah_fb_skin_006", "b_c_gah_fb_skin_007", "b_c_gah_fb_skin_008",
        "b_c_gah_fb_skin_009", "b_c_gah_fb_skin_010", "b_c_gan_fb_skin_jondar"
    },
    // humangirl
    {
        "b_c_gah_fg_skin_001", "b_c_gah_fg_skin_002", "b_c_gah_fg_skin_003", "b_c_gah_fg_skin_004",
        "b_c_gah_fg_skin_005", "b_c_gah_fg_skin_006", "b_c_gah_fg_skin_007", "b_c_gah_fg_skin_008",
        "b_c_gah_fg_skin_009", "b_c_gah_fg_skin_010", "b_c_gan_fg_skin_lyssa"
    },
    // dwarf
    {
        "b_c_gan_df_skin_01", "b_c_gan_df_skin_02", "b_c_gan_df_skin_03",
        "b_c_gan_df_skin_04", "b_c_gan_df_skin_05", "b_c_gan_df_skin_06"
    },
    // skeleton
    {
        "b_c_ecm_sk", "b_c_ecm_sk-02", "b_c_ecm_skg"
    },
    // utraean_boy
    {
        "b_c_gan_utraean_skin_01", "b_c_gan_utraean_skin_02", "b_c_gan_utraean_skin_03"
    },
    // utraean_girl
    {
        "b_c_gan_utraean_skin_04", "b_c_gan_utraean_skin_06"
    },
    // halftgiant
    {
        "b_c_gan_hg_skin_01", "b_c_gan_hg_skin_02", "b_c_gan_hg_skin_03",
        "b_c_gan_hg_skin_04", "b_c_gan_hg_skin_05", "b_c_gan_hg_skin_06",
        "b_c_gan_hg_skin_07"
    }
};

std::vector<std::vector<std::string>> hairStrings = {
    // humanboy
    {
        "b_c_gah_fb_hair_001", "b_c_gah_fb_hair_002", "b_c_gah_fb_hair_003", "b_c_gah_fb_hair_004",
        "b_c_gah_fb_hair_005", "b_c_gah_fb_hair_006", "b_c_gah_fb_hair_007", "b_c_gah_fb_hair_008",
        "b_c_gah_fb_hair_009", "b_c_gah_fb_hair_010"
    },
    // humangirl
    {
        "b_c_gah_fg_hair_001", "b_c_gah_fg_hair_002", "b_c_gah_fg_hair_003", "b_c_gah_fg_hair_004",
        "b_c_gah_fg_hair_005", "b_c_gah_fg_hair_006", "b_c_gah_fg_hair_007", "b_c_gah_fg_hair_008",
        "b_c_gah_fg_hair_009", "b_c_gah_fg_hair_010"
    },
    // dwarf
    {
        "default"// empty
    },
    // skeleton
    {
        "default"// empty
    },
    // utraean_boy
    {
        "b_c_gah_fb_hair_001", "b_c_gah_fb_hair_002", "b_c_gah_fb_hair_003", "b_c_gah_fb_hair_004",
        "b_c_gah_fb_hair_005", "b_c_gah_fb_hair_006", "b_c_gah_fb_hair_007", "b_c_gah_fb_hair_008",
        "b_c_gah_fb_hair_009", "b_c_gah_fb_hair_010"
    },
    // utraean_girl
    {
        "default",
        "b_c_gah_fg_hair_001", "b_c_gah_fg_hair_002", "b_c_gah_fg_hair_003", "b_c_gah_fg_hair_004",
        "b_c_gah_fg_hair_005", "b_c_gah_fg_hair_006", "b_c_gah_fg_hair_007", "b_c_gah_fg_hair_008",
        "b_c_gah_fg_hair_009", "b_c_gah_fg_hair_010"
    },
    // halftgiant
    {
        "default",
        "b_c_gah_fb_hair_001", "b_c_gah_fb_hair_002", "b_c_gah_fb_hair_003", "b_c_gah_fb_hair_004",
        "b_c_gah_fb_hair_005", "b_c_gah_fb_hair_006", "b_c_gah_fb_hair_007", "b_c_gah_fb_hair_008",
        "b_c_gah_fb_hair_009", "b_c_gah_fb_hair_010"
    }
};

std::vector<std::vector<std::string>> shirtStrings = {
    // humanboy
    {
        "b_c_pos_a1_jondar",
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    },
    // humangirl
    {
        "b_c_pos_a1_lyssa",
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    },
    // dwarf
    {
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    },
    // skeleton
    {
        "default"// empty
    },
    // utraean_boy
    {
        "b_c_pos_a1_utraean-01",
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    },
    // utraean_girl
    {
        "b_c_pos_a1_utraean-01", "b_c_pos_a1_utraean-04",
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    },
    // halftgiant
    {
        "b_c_pos_a1_100", "b_c_pos_a1_200", "b_c_pos_a1_300",
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    }
};

std::vector<std::vector<std::string>> pantsStrings = {
    // humanboy
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    },
    // humangirl
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    },
    // dwarf
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    },
    // skeleton
    {
        "default" // empty
    },
    // utraean_boy
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    },
    // utraean_girl
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    },
    // halftgiant
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    }
};

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

void GoDb::LoadSqlDb()
{
    std::atomic<bool> finished = false;
    std::atomic<int> pending = 0;

    std::vector<std::map<std::string, std::string>> rows;
    if (!g_engine.Db().QuerySync(
        "SELECT go_id, template_name, pcontent_query FROM t_gos;", rows))
    {
        std::cout << "Failed to query t_gos\n";
        return;
    }

    pending = static_cast<int>(rows.size());
    if (pending == 0)
        return;

    for (const auto& row : rows)
    {
        uint32_t goid = std::stoul(row.at("go_id"));
        Go* go = nullptr;

        if (auto* tpl = manager.GetTemplate(row.at("template_name")))
            go = new Go(*tpl, goid, row.at("pcontent_query"));

        if (go == nullptr)
        {
            cout << "Skipping go because template " << row.at("template_name") << " wasnt found in manager" << endl;
        }

        go->LoadFromDatabase(
            g_engine.Db(),
            [this, go, &pending, &finished](Go*)
            {
                // enqueue world registration
                g_engine.m_mainThreadJobs.push([this, go]()
                    {
                        godb.AddGo(go);

                        if (g_engine.IsPlayer(go) || go->IsItem() || go->Goid() == 5)
                        {
                            const std::string& region = go->Placement()->GetRegion();
                            if (!region.empty())
                            {
                                SendWorldMessage(we_entered_world, go, go, region);
                                std::cout << "[GODB] Spawned Go "
                                    << go->Goid() << "\n";
                            }
                        }

                        if (go->IsItem())
                            g_engine.RegisterItem(go);
                        else
                        {
                            g_engine.RegisterPlayerCharacter(go);
                        }
                    });

                if (--pending == 0)
                    finished = true;
            });
    }

    while (!finished)
    {
        g_engine.Loop();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void GoDb::SpawnGo(const string& templateName, const Go* summonerGo, const string& pContent)
{
    try
    {
        Go* t = godb.CloneGo(templateName, pContent);

        if (t == NULL)
        {
            Log::Write(Log::Level::ERR,
                "Unknown template: " + templateName, true);
            return;
        }

        t->CopyPlacement(summonerGo->Placement());

        string region = t->Placement()->GetRegion();
        if (!region.empty())
        {
            SendWorldMessage(we_entered_world, t, t, region);
            cout << "[GODB] Spawned Go " << t->Goid() << " using template: " << templateName << " in region: " << region << " at: " << t->Placement()->Position().X << " | " << t->Placement()->Position().Y << " | " << t->Placement()->Position().Z << " in node: " << t->Placement()->Position().Node << endl;
            if (t->IsItem())
            {
                g_engine.RegisterItem(t);
                t->Save(g_engine.Db());
                //t->SaveToXml("items");
            }
        }
    }
    catch (exception& e)
    {
        Log::WriteF(Log::Level::ERR, "go %u was not loaded because: %s", NextId(), e.what());
    }
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

        try
        {
            uint32_t id = NextId();
            Go* go = new Go(it->second, id);

            // --------------------------------------------
            // Check if the node exists in the region
            // --------------------------------------------
            auto region = g_world.GetRegion(go->Placement()->GetRegion());
            if (!region)
            {
                Log::Write(Log::Level::ERR,
                    "Region '" + go->Placement()->GetRegion() +
                    "' does not exist for template '" + it->second.name + "'", true);

                it = maps.erase(it);
                delete go;
                continue;
            }

            const auto& nodes = region->GetNodes();
            if (nodes.find(go->Placement()->Position().Node) == nodes.end())
            {
                // Region has no such node ? skip spawn

                /*Log::Write(Log::Level::ERR,
                    "Node '" + to_string(go->Placement()->Position().Node) +
                    "' does not exist for template '" + it->second.name + "'", true);*/

                it = maps.erase(it);
                delete go;
                continue;
            }

            string regionName = go->Placement()->GetRegion();
            if (!regionName.empty())
            {
                m_godb[id] = go;
                SendWorldMessage(we_entered_world, go, go, regionName);

                cout << "[GODB] Spawned Go " << id
                    << " using SCID template: " << it->second.name
                    << " in region: " << go->Placement()->GetRegion()
                    << " at: " << go->Placement()->Position().X
                    << " | " << go->Placement()->Position().Y
                    << " | " << go->Placement()->Position().Z
                    << " node: " << go->Placement()->Position().Node
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

    /*if (TemplateData* tpl = manager.GetTemplate("dso_utraean_boy"))
    {
        cout << "[info] template: " << tpl->name << "\n";
        if (!tpl->specializes.empty())
            cout << "  specializes: " << tpl->specializes << "\n";

        for (const auto& [compname, comp] : tpl->components) {
            gas.LogComponent(compname, comp, "  ");
        }
    }*/

    auto& all = manager.GetAll();
    for (auto it = all.begin(); it != all.end(); )
    {
        it = all.erase(it);
    }
    //delete &manager.templates;
}

void GoDb::AddGo(Go* go)
{
    m_godb[go->Goid()] = go;
}

void GoDb::RemoveGo(Go* go)
{
    if (!go)
        return;

    auto it = m_godb.find(go->Goid());
    if (it != m_godb.end())
        m_godb.erase(it);
}
void GoDb::LoadContentDb()
{
    size_t totalLoaded = 0;

    auto& maps = manager.GetAll();

    for (auto it = maps.begin(); it != maps.end(); it++ )
    {
        // SCID template name == base template name
        TemplateData* tpl = manager.GetTemplate(it->second.name);

        map<string, Go*>::iterator iterator = m_contentdb.find(tpl->name);
        if (iterator != m_contentdb.end())
        {
            throw runtime_error("template already exists in contentdb");
        }

        if (tpl->name.empty() != true)
        {
            try
            {
                Go* t = new Go(*tpl);
                m_contentdb[tpl->name] = t;
                //cout << "Loaded template " << tpl->name.c_str() << endl;
                totalLoaded++;
            }
            catch (exception& e)
            {
                Log::WriteF(Log::Level::ERR, "template %s was not loaded because : %s", tpl->name.c_str(), e.what());
            }
        }
    }

    Log::Write(Log::Level::INFO, "[INFO] Finished loading templates into contentDb. Total loaded: " + to_string(totalLoaded), true);
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

Go* GoDb::CloneGo(const string& template_name, const string& pcontent_query)
{
    map<string, Go*>::iterator iterator = m_contentdb.find(template_name);
    if (iterator != m_contentdb.end())
    {
        uint32_t id = NextId();

        Go* go = new Go(iterator->second, id, pcontent_query);
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

void GoDb::CreateChar(
	Account* account,
	const std::string& charName,
	int charType,
	int charHead,
	int charSkin,
	int charHair,
	int charShirt,
	int charPants,
	std::function<void()> onInserted
)
{
	// Validate indices
	if (charType < 0 || charType >= static_cast<int>(modelStrings.size()) ||
		charHead < 0 || charHead >= static_cast<int>(headStrings[charType].size()) ||
		charSkin < 0 || charSkin >= static_cast<int>(skinStrings[charType].size()) ||
		charHair < 0 || charHair >= static_cast<int>(hairStrings[charType].size()) ||
		charPants < 0 || charPants >= static_cast<int>(pantsStrings[charType].size()) ||
		charShirt < 0 || charShirt >= static_cast<int>(shirtStrings[charType].size()))
	{
		std::cout << "Invalid character customization index." << std::endl;
		return;
	}

	std::string strModel = modelStrings[charType];
	std::string strHead = headStrings[charType][charHead];
	std::string strSkin = skinStrings[charType][charSkin];
	std::string strHair = hairStrings[charType][charHair];
	std::string strPants = pantsStrings[charType][charPants];
	std::string strShirt = shirtStrings[charType][charShirt];

	uint32_t goid = godb.NextId();

	g_engine.Db().InsertGo(goid, charName, strModel, strHead, strSkin, strHair, strShirt, strPants, account,
		[this, account, strModel, onInserted]()
		{
			cout << "Go fully inserted to db\n";
			Go* go = godb.CloneGo(strModel);
			go->LoadFromDatabase(g_engine.Db(), [this, go, account, onInserted](Go* go)
				{
					g_engine.m_mainThreadJobs.push([this, go, account, onInserted]()
						{
							godb.AddGo(go);
							go->SetAccount(account);
							go->GetAccount()->AddCharacter(go);
							g_engine.RegisterPlayerCharacter(go);

							if (onInserted)
								onInserted();
						});
				});
		}
	);
}

void GoDb::DeleteChar(int selectSlot, Account* account, std::function<void()> onInserted)
{
	if (selectSlot < 1 || selectSlot > 8)
		return;

	if (!account)
		return;

	std::string query =
		"SELECT "
		"char_id_1, char_id_2, char_id_3, char_id_4, "
		"char_id_5, char_id_6, char_id_7, char_id_8 "
		"FROM t_accounts WHERE account_id = " +
		std::to_string(account->Id());

	g_engine.Db().AsyncQuery(query,
		[this, account, selectSlot, onInserted](const auto& rows)
		{
			if (rows.empty())
				return;

			const auto& row = rows[0];

			// Collect existing characters in logical order
			std::vector<uint32_t> chars;
			chars.reserve(8);

			for (int i = 1; i <= 8; ++i)
			{
				auto it = row.find("char_id_" + std::to_string(i));
				if (it != row.end() && !it->second.empty())
					chars.push_back(static_cast<uint32_t>(std::stoul(it->second)));
			}

			if (selectSlot > static_cast<int>(chars.size()))
				return;

			// Character to delete
			uint32_t goId = chars[selectSlot - 1];
			chars.erase(chars.begin() + (selectSlot - 1));

			// In-memory cleanup (safe ONLY if you already block further saves)
			if (Go* go = godb.FindGoById(goId))
			{
				account->RemoveCharacter(selectSlot);
				godb.RemoveGo(go);
				delete go;
			}

			// Build UPDATE to close the gap
			std::ostringstream update;
			update << "UPDATE t_accounts SET ";

			for (int i = 1; i <= 8; ++i)
			{
				update << "char_id_" << i << " = ";
				if (i <= static_cast<int>(chars.size()))
					update << chars[i - 1];
				else
					update << "NULL";

				if (i < 8)
					update << ", ";
			}

			update << " WHERE account_id = " << account->Id();

			// 1) Update account slots
			g_engine.Db().AsyncQuery(update.str(),
				[this, goId, onInserted](const auto&)
				{
					// 2) Delete Go (FK cascade)
					std::string delGo =
						"DELETE FROM t_gos WHERE go_id = " +
						std::to_string(goId);

					g_engine.Db().AsyncQuery(delGo,
						[onInserted](const auto&)
						{
							// 3) Main-thread callback
							g_engine.m_mainThreadJobs.push(
								[onInserted]()
								{
									if (onInserted)
										onInserted();
								});
						});
				});
		});
}