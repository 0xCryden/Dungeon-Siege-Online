#include "../Engine.hpp"
#include "WorldMessage.h"

WorldMessage :: WorldMessage (eWorldEvent event, Go * from, Go * to, const string & data) : m_WorldEvent (event), m_SendFrom (from), m_SendTo (to), m_Data (data) 
{
}

eWorldEvent WorldMessage :: WorldEvent () const
{
	return m_WorldEvent;
}

Go * WorldMessage :: SendFrom () const
{
	return m_SendFrom;
}

Go * WorldMessage :: SendTo () const
{
	return m_SendTo;
}

string WorldMessage :: Data () const
{
	return m_Data;
}

void SendWorldMessage (eWorldEvent event, Go * from, Go * to, const string & data)
{
	g_engine.HandleWorldMessage (WorldMessage (event, from, to, data));
}
