
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
    // Add initialization code here (e.g., setting up resources, logging, etc.)
    //std::cout << "Initialization complete.\n";
}

void Cleanup()
{
    // Add cleanup code here (e.g., releasing resources, logging, etc.)
    //std::cout << "Cleanup complete.\n";
}

	