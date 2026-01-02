#pragma once
#include "Common.h"
#include "map/WorldMap.h"
#include "events/Event.hpp"
#include "msg/WorldMessage.h"
#include <queue>
	
class Engine
{
	public:
		Engine ();
		~Engine ();

		bool IsRunning() const;
		void Loop();
		void RegisterEvent(Event* event);
		void HandleWorldMessage(const WorldMessage& message);

		void RegisterPlayer(Go* go);  // Add this
		void UnregisterPlayer(Go* go); // Optional, for logout cleanup
		bool IsPlayer(Go* obj) const;

		void RegisterItem(Go* go);  // Add this
		void UnregisterItem(Go* go); // Optional, for logout cleanup

		void RegisterPlayerCharacter(Go* go);
		void UnregisterPlayerCharacter(Go* go);

		GopSet GetPlayers() { return m_players; };
		GopSet GetPlayerCharacters() { return m_playerChars; };
		GopSet GetItems() { return m_items; };

		void UpdateGo(Go* go, eWorldEvent type, const string& data = "");
		void UpdateGoExp(Go* go, double value);
		void UpdateGoLvlup(Go* go, const string & data);

		std::queue<std::function<void()>> m_mainThreadJobs;
		MySQL& Db() { return m_db; }

	private:
		void TimerPerSecond();
		void TimerPerMinute();
		void TimerPerHour();

		void MessageKnown (Go * go, const WorldMessage & message);
		void MessageAllPlayers (const WorldMessage & message);
		void AddGoToRegion (Go * go, const string & data);
		void RemoveGoFromRegion (Go * go);
		void CalculateFrustums ();
			
		bool m_Running;
		std::priority_queue<Event *, std::vector<Event *>, EventComparison> m_EventRegistry;
		GopSet m_players;
		GopSet m_playerChars;
		GopSet m_items;
		MySQL m_db;
};
	
extern Engine g_engine;