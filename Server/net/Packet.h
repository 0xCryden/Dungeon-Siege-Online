#pragma once
//#include "../platform/Platform.h"
#include <string>
#include <vector>
#include <cstdint>
#include "../Common.h"
	
//using namespace std;
	
class Packet
{
	public:			
		Packet (); // outgoing packet
		Packet (const char * buffer); // incoming packet
		~Packet ();

		void WriteUInt64 (uint64_t data);
		void WriteInt64(int64_t data);
		void WriteUInt32 (uint32_t data);
		void WriteInt32 (int32_t data);
		void WriteUInt16 (uint16_t data);
		void WriteInt16 (int16_t data);
		void WriteUInt8 (uint8_t data);
		void WriteInt8 (int8_t data);
		void WriteFloat(float data);
		void WriteDouble(double data);
		void WriteString (const string & buffer);

		uint64_t ReadUInt64();
		int64_t ReadInt64();
		uint32_t ReadUInt32();
		int32_t ReadInt32();
		uint16_t ReadUInt16 ();
		int16_t ReadInt16 ();
		uint8_t ReadUInt8 ();
		int8_t ReadInt8 ();
		float ReadFloat();
		double ReadDouble();
		string ReadString ();
			
		uint16_t Size ();
		const char * Data ();
			
	private:
			
		void Append (const char * buffer, uint16_t size);
			
		uint16_t m_size;
		uint16_t m_position;
		char* m_data;
		//vector<char> m_data;
};
