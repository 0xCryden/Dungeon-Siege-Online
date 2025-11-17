#include "Engine.hpp" // extern engine
#include "net/Network.hpp"
#include "server/Server.hpp"
#include "events/SendWorldMessageEvent.hpp"
#include "helper/Helper.h"
#include "platform/windows/WinSockApi.h"
#include "xml/XmlCleanupGuard.h"
#include <random>
#include "gas/Gas.hpp"

int main(int argc, char** argv)
{
    try 
    {
        Log::Init("log.txt");
        Log::Write(Log::Level::INFO, "", true);
        Log::Write(Log::Level::INFO, "    ###   DungeonSiegeOnline v0.5   ###", true);
        Log::Write(Log::Level::INFO, "", true);
        // Seeding with modern RNG
        random_device rd;
        mt19937 rng(rd());

        // System guards
        WinSockApi wsaData;
        XmlCleanupGuard xmlGuard;

        // --- Network Init ----------------------------------------------------
        Network network;
        network.Bind(4000);

        // --- Resource Loading ------------------------------------------------
        /*
         * load resources in the following order :
         * map, items, items which can hold items, actors, players
         */
        const string dataDir = "data";
        g_world.LoadAllMaps();
        gas.LoadTemplates();
        gas.LoadMapTemplates();
        // TODO dont load into m_contentdb but m_godb (ínstances instead of templates)
        //godb.LoadGasToGo();

        if (TemplateData* tpl = manager.GetTemplate("bd_ch_f_g_c_avg"))
        {
            cout << "[info] template: " << tpl->name << "\n";
            if (!tpl->specializes.empty())
                cout << "  specializes: " << tpl->specializes << "\n";
        
            for (const auto& [compname, comp] : tpl->components) {
                gas.LogComponent(compname, comp, "  ");
            }
            // access fields
            /*auto* comp = tpl->getcomponent("aspect");
            if (comp) {
                auto experience_value = comp->getfield("experience_value");
                if (experience_value) {
                    std::cout << "experience_value: " << *experience_value << "\n";
                }
            }*/
        }

        // Load specific GO folders & content
        godb.LoadGoDbFolder("items");
        godb.LoadGoDbFolder("actors");
        godb.LoadContentDb(dataDir + "\\static\\actors.xml");
        server.LoadAccounts(dataDir + "\\dynamic\\accounts.xml");

        godb.GasToGoDb();

        PostWorldMessage(we_frustum_active_state_changed, 0, 0, "", 2500);
        
        while (g_engine.IsRunning()) {
            network.Listen();
            g_engine.Loop();
            sleep_microseconds(1000);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal std::exception: " << e.what() << "\n";
        std::ofstream log("log.txt");
        log << "Fatal std::exception: " << e.what() << "\n";
        return -1;
    }
    catch (int e) {
        std::cerr << "Fatal int exception: " << e << "\n";
        std::ofstream log("log.txt");
        log << "Fatal int exception: " << e << "\n";
        return e;
    }
    catch (...) {
        std::cerr << "Fatal unknown exception\n";
        std::ofstream log("log.txt");
        log << "Fatal unknown exception\n";
        return -1;
    }

    return 0;
}
