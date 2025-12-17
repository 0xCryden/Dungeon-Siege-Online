#include "server/Server.hpp"
#include "helper/Log.h"

int main(int argc, char** argv)
{
    try 
    {
        Log::Init();
        // TODO implement configuration
        //Config config = Config::loadFromFile("server.cfg");
        Server server; // Server server(config);
        server.Start();
        server.Loop();
        server.Stop();
        Log::Flush();
        return EXIT_SUCCESS;
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
/*if (TemplateData* tpl = manager.GetTemplate("bd_ch_f_g_c_avg"))
{
    cout << "[info] template: " << tpl->name << "\n";
    if (!tpl->specializes.empty())
        cout << "  specializes: " << tpl->specializes << "\n";

    for (const auto& [compname, comp] : tpl->components) {
        gas.LogComponent(compname, comp, "  ");
    }
}*/