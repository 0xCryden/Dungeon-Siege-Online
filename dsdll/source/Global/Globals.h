
#ifndef _GLOBALS_H_
#define _GLOBALS_H_

	// Include Libraries
	#include <string>
	#include <time.h>
	#include "../GPG.h"
	

#define BUFFER_SIZE 1024

struct PlayerId_;
typedef const PlayerId_* PlayerId;

	// Globals

	using namespace std;

	enum eEquipSlot
	{
		ES_SHIELD_HAND = 0,
		ES_WEAPON_HAND,
		ES_FEET,
		ES_CHEST,
		ES_HEAD,
		ES_FOREARMS,
		ES_AMULET,
		ES_SPELLBOOK,
		ES_RING_0,
		ES_RING_1,
		ES_RING_2,
		ES_RING_3,
		ES_RING,
		ES_NONE,
		ES_ANY,

	};

	enum eInventoryLocation
	{
		il_active_melee_weapon = 0,
		il_active_ranged_weapon = 1,
		il_active_primary_spell = 2,
		il_active_secondary_spell = 3,
		il_spell_1 = 4,
		il_spell_2 = 5,
		il_spell_3 = 6,
		il_spell_4 = 7,
		il_spell_5 = 8,
		il_spell_6 = 9,
		il_spell_7 = 10,
		il_spell_8 = 11,
		il_spell_9 = 12,
		il_spell_10 = 13,
		il_spell_11 = 14,
		il_spell_12 = 15,
		il_shield = 16,
		il_invalid = 17,
		il_all = 18,
		il_all_active = 19,
		il_all_spells = 20,
		il_main = 21,
	};

	enum eActorAlignment
	{
		AA_GOOD = 0,
		AA_NEUTRAL,
		AA_EVIL,
	};

	enum eLifeState
	{
		LS_IGNORE = 0,
		LS_ALIVE_CONSCIOUS,
		LS_ALIVE_UNCONSCIOUS,
		LS_DEAD_NORMAL,
		LS_DEAD_CHARRED,
		LS_DEAD_FRESH,
		LS_DEAD_BONES,
		LS_DEAD_DUST,
		LS_GONE,
		LS_GHOST,
	};

	// Returns the current system hour (0-23)
	FEX int GetCurrentHour() {
		time_t now = time(0);
		tm* localTime = localtime(&now);
		return localTime->tm_hour;
	}

	// Returns the current system minute (0-59)
	FEX int GetCurrentMinute() {
		time_t now = time(0);
		tm* localTime = localtime(&now);
		return localTime->tm_min;
	}

	FEX float Atan2(float y, float x) {
		return atan2f(y, x);
	}

	FEX void Log(const char* format, ...) {
		char buffer[BUFFER_SIZE];

		va_list args;
		va_start(args, format);
		vsnprintf(buffer, BUFFER_SIZE, format, args);
		va_end(args);

		std::ofstream logfile("ds_log.txt", std::ios::app);
		if (logfile.is_open()) {
			logfile << buffer << std::endl;
		}
	};

	FEX void LogInline(const char* format, ...) {
		char buffer[BUFFER_SIZE];

		va_list args;
		va_start(args, format);
		vsnprintf(buffer, BUFFER_SIZE, format, args);
		va_end(args);

		std::ofstream logfile("ds_log.txt", std::ios::app);
		if (logfile.is_open()) {
			logfile << buffer;
		}
	};

	FEX eEquipSlot MakeEquipSlot (int es)
	{
		return (eEquipSlot)es;
	}

	FEX eInventoryLocation MakeInvenLocation (int il)
	{
		return (eInventoryLocation)il;
	}

	FEX eActorAlignment MakeActorAlignment (int align)
	{
		return (eActorAlignment)align;
	}

	FEX eLifeState MakeLifeState (int ls)
	{
		return (eLifeState)ls;
	}

	FEX int MakeInt (eEquipSlot es)
	{
		return (int)es;
	}

	FEX int MakeInt (eInventoryLocation il)
	{
		return (int)il;
	}

	FEX int MakeInt (eActorAlignment align)
	{
		return (int)align;
	}

	FEX int MakeInt (eLifeState ls)
	{
		return (int)ls;
	}

	FEX int MakeInt(const char* string)
	{
		int value = 0;
		std::stringstream ss(string);
		ss >> value;
		return value;
	}

	FEX const char * MakeString(int value)
	{
		static std::string result; // must be static to keep memory alive
		std::stringstream ss;
		ss << value;
		result = ss.str();
		return result.c_str();
	}

	string m_strMemory;
	FEX const char * GenerateRandomString (unsigned int length)
	{
		srand ((unsigned) time(NULL));

		std::string random;
		char randomLetter;

		for (unsigned int i = 0; i < length; i++)
		{
				randomLetter = (char)(rand() % 26) + 'a';
				random += randomLetter;
		}

		m_strMemory = random;
		return (random.c_str());
	}

	class Game
	{
	public:
		FEX void STeleportPlayer(PlayerId playerId, const char* where, bool teleportSelectedItems)
		{
			static unsigned int Address_Init = 0;
			static unsigned int Address_STeleportPlayer = 0;

			Log("Game::STeleportPlayer( %s, %d)\n", where, teleportSelectedItems);

			if (!Address_STeleportPlayer)
			{
				if (Address_Init) return;
				Log("Assigning address");
				Address_STeleportPlayer = 0x0040A597;  // dsmod
				//Address_STeleportPlayer = 0x0040A127; // dsretail
				// Check first 4 bytes of function prologue for signature (opcode bytes)
				// Function prologue: push ebp; mov ebp, esp; sub esp, 0x20
				// In hex: 55 8B EC 83 EC 20 -> DWORDs: 0x83EC8B55 (little-endian)
				/*if (*((unsigned long*)Address_STeleportPlayer) != 0x83EC8B55)
				{
					Address_STeleportPlayer = 0;
					Log("  unable to find Game::STeleportPlayer()\n");
				}*/
			}

			Address_Init = 1;
			if (!Address_STeleportPlayer) return;

			Log("Executing asm");
			__asm
			{
				movzx   eax, teleportSelectedItems
				push    eax
				push    where
				push    playerId
				mov     ecx, this
				call    Address_STeleportPlayer
			}
		}
		/*FEX void STeleportPlayer(int playerId, const char* where, bool teleportSelectedItems)
		{
			static unsigned int Address_Init = 0;
			static unsigned int Address_STeleportPlayer = 0;

			if (!Address_STeleportPlayer)
			{
				if (Address_Init) return;

				Address_STeleportPlayer = 0x004AF31A; // Replace with the correct address if needed

				// Optional signature check (if you have one)
				if (*((unsigned long*)Address_STeleportPlayer) != 0x558BEC83) // ds 1.1 first 4 bytes
				{
					Address_STeleportPlayer = 0;
					Log("  unable to find TattooGame::STeleportPlayer()\n");
				}
			}

			Address_Init = 1;
			if (!Address_STeleportPlayer) return;

			Log("Calling STeleportPlayer(%d, %s, %d)\n", playerId, where, teleportSelectedItems);

			void* thisPtr = this;//GetTattooGame(); // you must implement this!

			if (!thisPtr)
			{
				Log("TattooGame global is null!\n");
				return;
			}

			__asm
			{
				mov     eax, teleportSelectedItems
				push    eax; bool teleportSelectedItems

				mov     eax, where
				push    eax; const char* where

				mov     eax, playerId
				push    eax; PlayerId

				call    Address_STeleportPlayer
			}
		}*/
	};

#endif