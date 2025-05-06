/*
 *  This file is part of dsmmorpg.
 *  
 *  dsmmorpg is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
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

#include "Engine.hpp"
#include "net/Network.hpp"
#include "map/WorldMap.hpp"
#include "server/Server.hpp"
#include "events/SendWorldMessageEvent.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <winsock2.h>
#include <windows.h>

using namespace std;

int main(int argc, char** argv)
{
    try {
        cout << "dsmmorpg v1.01" << endl;

        srand(time(NULL));

        // Initialize Windows Sockets (WSA)
        WSADATA wsaData;
        int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaResult != 0) {
            cout << "WSAStartup failed with error: " << wsaResult << endl;
            return -1;
        }

        // Construct network inside main to avoid static/global issues
        Network network;

        /*
         * catch any standard socket errors
         */
        try {
            network.Bind(4000);
        }
        catch (int& e) {
            cout << "fatal network error : " << e << endl;
            WSACleanup();
            return -1;
        }

        /*
         * load resources in the following order :
         * map, items, items which can hold items, actors, players
         */
        try {
            string dataDir = "data";

            world.LoadMap(dataDir + "\\static\\map\\main.xml");

            godb.LoadGoDb(dataDir + "\\dynamic\\items.xml");
            godb.LoadGoDb(dataDir + "\\dynamic\\actors.xml");

            // godb.LoadContentDb(dataDir + "\\static\\items.xml");
            godb.LoadContentDb(dataDir + "\\static\\actors.xml");

            server.LoadAccounts(dataDir + "\\dynamic\\accounts.xml");
        }
        catch (exception& e) {
            cout << "exception caught : " << e.what() << endl;
            xmlCleanupParser();
            WSACleanup();
            return -1;
        }

        try {
            PostWorldMessage(we_frustum_active_state_changed, 0, 0, "", 2500);
            while (engine.IsRunning()) {
                network.Listen();
                engine.Loop();
                usleep(1);
            }
        }
        catch (exception& e) {
            cout << "exception caught : " << e.what() << endl;
        }
        catch (int& e) {
            cout << "network error caught : " << e << endl;
        }

		std::cout << "Start Cleanup... " << std::endl;
        WSACleanup();
        xmlCleanupParser();
    }
    catch (int e) {
        cerr << "Unhandled int exception: " << e << endl;
        ofstream log("log.txt");
        log << "Unhandled int exception: " << e << endl;
        return e;
    }
    catch (exception& e) {
        cerr << "Unhandled std::exception: " << e.what() << endl;
        ofstream log("log.txt");
        log << "Unhandled std::exception: " << e.what() << endl;
        return -1;
    }
    catch (...) {
        cerr << "Unhandled unknown exception" << endl;
        ofstream log("log.txt");
        log << "Unhandled unknown exception" << endl;
        return -1;
    }

    return 0;
}
