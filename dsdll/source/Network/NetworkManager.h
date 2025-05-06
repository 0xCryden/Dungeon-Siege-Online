
#ifndef _NETMANAGER_H_
#define _NETMANAGER_H_

	// Include Libraries

	#include <vector>
	#include <algorithm>
	#include <stdio.h>
	#include <string>

	#include "../GPG.h"
	#include "wsa.h"
	#include "Socket.h"
	#include "Packet.h"

	#include <fstream>
	#include <ctime>
	#include <sstream>  // Add this if you use stringstream

/*void LogToFile(const char* message)
{
	std::ofstream log("dsdll_log.txt", std::ios::app); // Append mode
	if (log.is_open())
	{
		std::time_t now = std::time(nullptr);
		log << std::ctime(&now) << " " << message << std::endl;
		log.close();
	}
}*/
const char* GetPacketName(u_int8_t type, bool isClientToServer) {
	switch (isClientToServer) {
	case true: {
		switch (type) {
			// if (isServerToClient == true) {
		case 1: return "RSAUTHENTICATION";
		case 2: return "RSPLAYERREADY";
		case 11: return "RSCHAT";
		case 12: return "RSMOVE";
		case 20: return "RSGET";
		case 21: return "RSDROP";
		case 22: return "RSEQUIP";
		case 23: return "RSUNEQUIP";
		case 24: return "RSATTACKMELEE";
		case 25: return "RSATTACKRANGED";
		case 26: return "RSCAST";
		case 40: return "RSJOBTRAVELDISTANCEREACHED";

		default: return "UNKNOWN";
		}
	}
	default: {
		switch (type) {
		case 1: return "RCAUTHENTICATION";
		case 6: return "RCCREATEACTOR";
		case 7: return "RCCREATEITEM";
		case 9: return "RCDISPLAYMESSAGE";
		case 10: return "RCCREATEGO";
		case 11: return "RCCHAT";
		case 12: return "RCMOVE";
		case 13: return "RCDESTROYGO";
		case 14: return "RCSETSCREENHERO";
		case 20: return "RCGET";
		case 21: return "RCDROP";
		case 22: return "RCEQUIP";
		case 23: return "RCUNEQUIP";
		case 24: return "RCATTACKMELEE";
		case 25: return "RCATTACKRANGED";
		case 27: return "RCAPPROACH";

		default: return "UNKNOWN";
		}
	}
	}
}

	void midcopy ( char * p_input, char * p_output, int p_start, int p_stop )
	{
		int p_index = 0;

		for ( int i = p_start; i < p_stop; i++ )
		{
			p_output[p_index] = p_input[i];
			p_index++;
		}

		p_output[p_index] = '\0';
	}

	class NetworkManager : public Socket, public Singleton <NetworkManager>
	{
		public:

			#define gNetworkManager NetworkManager::GetSingleton ();

			NetworkManager ()
			{
				std::ostringstream oss;
				oss << "[NetMan] Manager Constructed";
				LogToFile(oss.str().c_str());
			}

			~NetworkManager ()
			{
				ClearAllPackets ();
				Socket::Close ();
			}

			FEX int Connect (const char * ip, Port port)
			{
				std::ostringstream oss;
				oss << "[Connect] Trying to connect to " << ip << ":" << port;
				LogToFile(oss.str().c_str());

				int error = Socket::Connect (GetIpAddress (ip), port);

				oss << "[Connect] Result: " << error;
				LogToFile(oss.str().c_str());

				if (error == -1)
				{
					return (-1);
				}
				Socket::SetBlocking (false);

				return (error);
			}

			FEX bool IsConnected ()
			{
				return (Socket::IsConnected());
			}

			FEX int Send (Packet * p_Packet)
			{
				const char* data = (const char*)p_Packet->Data();
				uint8_t packetType = (uint8_t)data[2]; // assuming size is at bytes 0–1

				std::ostringstream oss;
				oss << "[Send] Type: " << GetPacketName(packetType, true)
					<< "(id=" << static_cast<int>(packetType)
					<< "), Size: " << p_Packet->Size();
				LogToFile(oss.str().c_str());

				int bytes = ::send(m_Socket, data, p_Packet->Size(), 0);

				delete (p_Packet);

				return (bytes);
			}

			FEX int Receive(void)
			{
				if (!Socket::IsConnected())
				{
					LogToFile("[Receive] Not connected, returning -1.");
					return -1;
				}

				int count = 0;
				int bytes = recv(m_Socket, m_buffer, 8192, 0);

				/*std::ostringstream oss;
				oss << "[Receive] Bytes received: " << bytes;
				LogToFile(oss.str().c_str());*/
				std::ostringstream oss;

				if (bytes > 0)
				{
					int position = 0;

					u_int16_t size2;
					if (bytes > 2)
					{
						memcpy((unsigned char*)(&size2), m_buffer + position, sizeof(size2));
						u_int16_t size_net = size2;
						size2 = ntohs(size2);
					}

					std::ostringstream oss;
					uint8_t packetType = static_cast<uint8_t>(m_buffer[2]); 
					oss.str("");
					oss.clear();
					oss << "[Receive] Type: " << GetPacketName(packetType, false)
						<< " (id=" << static_cast<int>(packetType)
						<< "), Size: " << size2;
					LogToFile(oss.str().c_str());

					while (bytes > 2)
					{
						// Copy and convert size field
						u_int16_t size;
						memcpy((unsigned char*)(&size), m_buffer + position, sizeof(size));
						u_int16_t size_net = size;
						size = ntohs(size);

						/*oss.str("");
						oss.clear();
						oss << "[Receive] Raw size bytes: " << std::hex << ((unsigned int)m_buffer[position] & 0xFF)
							<< " " << ((unsigned int)m_buffer[position + 1] & 0xFF)
							<< " | Interpreted size: " << std::dec << size;
						LogToFile(oss.str().c_str());*/

						if (size == (u_int16_t)-1)
						{
							LogToFile("[Receive] Invalid packet size -1, returning -10.");
							return -10;
						}

						if (size <= bytes)
						{
							Packet* packet = new Packet((unsigned char*)(m_buffer + position));
							m_PacketStream.push_back(packet);

							/*oss.str("");
							oss.clear();
							oss << "[Receive] Accepted packet of size " << size << " at position " << position;
							LogToFile(oss.str().c_str());*/

							count++;
						}
						else
						{
							oss.str("");
							oss.clear();
							oss << "[Receive] Packet size " << size << " exceeds remaining bytes " << bytes << ", breaking.";
							LogToFile(oss.str().c_str());
							break;
						}

						bytes -= size;
						position += size;
					}

					return count;
				}
				else if (bytes == 0)
				{
					LogToFile("[Receive] Connection closed gracefully (recv == 0).");
				}
				else
				{
					//std::ostringstream err;
					//err << "[Receive] recv() returned error: " << WSAGetLastError();
					//LogToFile(err.str().c_str());
				}

				return 0;
			}


			FEX Packet * NextPacket ()
			{
				std::vector <Packet *>::iterator p_sIterator = m_PacketStream.begin();

				if (p_sIterator != m_PacketStream.end())
				{
					return (* p_sIterator);
				}
				
				return (NULL);
			}

			FEX Packet * CreatePacket ()
			{
				return (new Packet());
			}

			FEX void DiscardPacket (Packet * p_Packet)
			{
				std::vector <Packet *>::iterator p_sIterator = std::find (m_PacketStream.begin(), m_PacketStream.end(), p_Packet);

				if (p_sIterator != m_PacketStream.end())
				{
					delete (* p_sIterator);

					m_PacketStream.erase (p_sIterator);
				}
			}

			FEX void Disconnect ()
			{
				Socket::Close();
			}

		private:

			void ClearAllPackets ()
			{
				std::vector <Packet *>::iterator iterator;
				for (iterator = m_PacketStream.begin(); iterator != m_PacketStream.end(); iterator++)
				{
					delete (* iterator);
					m_PacketStream.erase (iterator);
				}
			}

			FUBI_SINGLETON_CLASS (NetworkManager, "Author(s): Sam Brkopac");

		protected:

			char m_buffer[8192];
			std::vector <Packet *> m_PacketStream;
	};

	NetworkManager g_NetworkManager;

#endif