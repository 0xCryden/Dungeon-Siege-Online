
	// Include Libraries

	#include "source/Network.h"

	#include "source/Go.h"

	#include "source/UI.h"

	#include "source/Map.h"

	#include "source/Global.h"

#include <Windows.h>
#include <iostream>

// Declare any initialization or cleanup functions for your DLL
extern void Initialize();
extern void Cleanup();

// DllMain entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Called when the DLL is loaded into a process
        //std::cout << "DLL Loaded\n";
        Initialize();  // Custom function to initialize resources, etc.
        break;

    case DLL_THREAD_ATTACH:
        // Called when a thread is created within the process
        break;

    case DLL_THREAD_DETACH:
        // Called when a thread is terminated within the process
        break;

    case DLL_PROCESS_DETACH:
        // Called when the DLL is unloaded from the process
        //std::cout << "DLL Unloaded\n";
        Cleanup();  // Custom function to clean up resources
        break;
    }
    return TRUE;
}
// Example functions for initialization and cleanup
void Initialize()
{
   /* HMODULE hExe = GetModuleHandleA(NULL); // DungeonSiege.exe

    // Resolve WorldMap singleton getter
    const char* singletonName = "?FUBI_GetClassSingleton@WorldMap@@CAPAV1@XZ";
    g_GetWorldMapSingleton = (FUBI_GetClassSingletonFunc)GetProcAddress(hExe, singletonName);
   if (!g_GetWorldMapSingleton) {
        Log("Failed to find WorldMap::FUBI_GetClassSingleton");
    }

    const char* regionFuncName = "?GetRegionNameForNode@WorldMap@@QBEABV?$gpbstring@DU?$char_traits@D@std@@V?$allocator@D@2@@@Vdatabase_guid@siege@@@Z";
    g_GetRegionNameForNode = (GetRegionNameForNodeFunc)GetProcAddress(hExe, regionFuncName);
   if (!g_GetRegionNameForNode) {
        Log("Failed to find WorldMap::GetRegionNameForNode");
    }
    else
        Log("Successfully resolved GetRegionNameForNode");*/
}

void Cleanup()
{
    // Add cleanup code here (e.g., releasing resources, logging, etc.)
    //std::cout << "Cleanup complete.\n";
}

	