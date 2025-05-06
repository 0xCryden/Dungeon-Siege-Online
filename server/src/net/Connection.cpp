/*
 *  This file is part of dsmmorpg.
 *  
 *  dsmmorpg is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
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

//  #include <winsock2.h>

#include "Connection.hpp"
#include "Packet.hpp"
#include <iostream> // For logging output

#ifdef _WIN32
 // #include <winsock2.h>
  //#include <ws2tcpip.h>
  //#include <windows.h>
  //#pragma comment(lib, "ws2_32.lib")
#else
  #include <fcntl.h>
  #include <unistd.h>
  #include <errno.h>
  #include <string.h> // For memcpy
  #include <netinet/in.h> // For ntohs
  #include <sys/socket.h> // For socket functions
#endif

// Logging utility
static void Log(const std::string& message) {
    std::cerr << "[Connection] " << message << std::endl;
}

Connection::Connection(int descriptor)
	: m_descriptor(descriptor), m_connected(false), m_state(NULL)
{
	if (m_descriptor != -1)
	{
		m_connected = true;
		Log("New connection created: descriptor = " + std::to_string(m_descriptor));
	}
}

void Connection::SetNonBlockingFlag(bool nonblocking)
{
#ifdef _WIN32
	u_long mode = nonblocking ? 1 : 0;
		if (ioctlsocket(m_descriptor, FIONBIO, &mode) != 0)
		{
			int err = WSAGetLastError();
			Log("Failed to set non-blocking mode (Windows): " + std::to_string(err));
			throw err;
		}
#else
	int flags = fcntl(m_descriptor, F_GETFL, 0);
	if (flags == -1)
	{
		throw errno;
	}
	
	if (nonblocking)
	{
		flags |= O_NONBLOCK;
	}
	else
	{
		flags &= ~O_NONBLOCK;
	}
	
	int error = fcntl(m_descriptor, F_SETFL, flags);
	if (error == -1)
	{
		throw errno;
	}
#endif
}

const char* GetPacketName(u_int8_t type, bool isServerToClient) {
    switch (isServerToClient) {
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
	    case 31: return "RSINVENMOVE";
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

int Connection::Send(const char* buffer, int size)
{
	// Add logging here
	if (size > 2) // make sure we can safely access data[2]
	    {
	        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(buffer);
	        uint8_t packetType = bytes[2];  // packet ID usually starts at byte 2
	        std::cout << "[SEND] Packet Type: " << GetPacketName(packetType, true)
					  << " (id=" << (int)packetType << "), Size: " << size << std::endl;
	    }

	int error = ::send(m_descriptor, buffer, size, 0);

	if (error == -1)
	{
#ifdef _WIN32
		int e = WSAGetLastError();
		if (e != WSAEWOULDBLOCK)
#else
		int e = errno;
		if (e != EWOULDBLOCK)
#endif
		{
			Log("Send failed: " + std::to_string(e));
			throw e;
		}
		error = 0;
	}

	return error;
}

int Connection::Receive()
{
	int error = ::recv(m_descriptor, m_buffer, 1024, 0);

	if (error == 0)
	{
		Log("Connection closed gracefully by peer.");
		throw errno;
	}
	if (error == -1)
	{
#ifdef _WIN32
		int e = WSAGetLastError();
#else
		int e = errno;
#endif
		Log("Receive failed: " + std::to_string(e));

#ifdef _WIN32
		if (e == WSAECONNRESET)
#else
		if (e == ECONNRESET)
#endif
		{
			m_connected = false;
			Log("Connection reset by peer.");
		}
		throw e;
	}

	if (m_state != NULL)
	{
		u_int16_t position = 0;
		while (error > 2)
		{
			u_int16_t size;
			memcpy((char*)(&size), m_buffer + position, sizeof(u_int16_t));
			size = ntohs(size);

			if (size == 0) break;

			if (size <= error)
			{
				Log("Packet received (size=" + std::to_string(size) + ")");
				m_state->Handle(m_buffer + position);
			}
			else
			{
				Log("Incomplete packet (expected=" + std::to_string(size) +
				    ", remaining=" + std::to_string(error) + ")");
			}

			error -= size;
			position += size;
		}
	}

	return error;
}

void Connection::Close()
{
	int error;
	if (m_connected)
	{
#ifdef _WIN32
		error = shutdown(m_descriptor, SD_BOTH);
#else
		error = shutdown(m_descriptor, 2);
#endif
		if (error == -1)
		{
#ifdef _WIN32
			Log("Shutdown failed: " + std::to_string(WSAGetLastError()));
			throw WSAGetLastError();
#else
			throw errno;
#endif
		}
	}

#ifdef _WIN32
	error = closesocket(m_descriptor);
#else
	error = ::close(m_descriptor);
#endif
	if (error == -1)
	{
#ifdef _WIN32
		Log("Close socket failed: " + std::to_string(WSAGetLastError()));
		throw WSAGetLastError();
#else
		throw errno;
#endif
	}
	Log("Connection closed: descriptor = " + std::to_string(m_descriptor));
	m_descriptor = -1;
	m_connected = false;

	if (m_state != NULL)
	{
		m_state->OnExit();
		delete m_state;
		m_state = NULL;
	}
}

WorldState* Connection::State() const
{
	return m_state;
}

void Connection::SetWorldState(WorldState* state)
{
	if (m_state != NULL)
	{
		m_state->OnExit();
		delete m_state;
		m_state = NULL;
	}

	m_state = state;
	m_state->OnEnter();
}
