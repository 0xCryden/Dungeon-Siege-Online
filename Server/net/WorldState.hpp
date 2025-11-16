#pragma once
#include "Packet.h"

class Connection;
	
class WorldState
{
	public:
		WorldState (Connection * client) : m_connection (client) {}
		virtual ~WorldState () {};
			
		virtual void OnEnter () = 0;
		virtual void Handle (const char * buffer) = 0;
		virtual void OnExit () = 0;
			
	protected:
		Connection * m_connection;
};
