#ifndef WORLDMESSAGE_H_
#define WORLDMESSAGE_H_

#include "../enum/eWorldEvent.hpp"
#include <string>
	
class WorldMessage
{
	public:
			
		WorldMessage (eWorldEvent event, Go * from, Go * to, const string & data);
			
		eWorldEvent WorldEvent () const;
			
		Go * SendFrom () const;
			
		Go * SendTo () const;
			
		string Data () const;
			
	private:
			
		eWorldEvent m_WorldEvent;
		Go * m_SendFrom;
		Go * m_SendTo;
		string m_Data;
};
	
void SendWorldMessage (eWorldEvent event, Go * from, Go * to, const string & data);

#endif /*WORLDMESSAGE_H_*/
