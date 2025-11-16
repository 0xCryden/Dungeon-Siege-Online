

	// Include Libraries

	#include "wsa.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <string>

	// wsa Class

	class wsa
	{
		public:

			wsa ( void )
			{
				// Start the winsock library
				WSAStartup ( MAKEWORD ( 2, 2 ), &m_wsaData );
			}

			~wsa ( void )
			{
				// Close the winsock library
				WSACleanup();
			}

		protected:

			WSADATA m_wsaData;
	};

	// Declare Exports

	wsa g_wsa;

	// Function Definitions

	bool IsIpAddress ( const char * p_strAddress )
	{
		for ( size_t p_iIterator = 0; p_iIterator < strlen ( p_strAddress ); p_iIterator++ )
		{
			if ( ( p_strAddress[p_iIterator] < '0' || p_strAddress[p_iIterator] > '9' ) && p_strAddress[p_iIterator] != '.' )
			{
				// Return a false on the ip address
				return ( false );
			}
		}

		// Return a true on the ip address
		return ( true );
	}

	//using Ip = uint32_t; // Assuming Ip is 32-bit IPv4

	Ip GetIpAddress(const char* p_strAddress)
	{
		if (!p_strAddress)
			throw std::invalid_argument("Null pointer passed to GetIpAddress");

		// Try to parse as dotted-decimal IPv4
		in_addr addr{};
		if (InetPtonA(AF_INET, p_strAddress, &addr) == 1)
		{
			// Valid IPv4 address
			return addr.S_un.S_addr;
		}

		// Otherwise, resolve hostname
		addrinfo hints{};
		hints.ai_family = AF_INET; // IPv4
		hints.ai_socktype = SOCK_STREAM;

		addrinfo* result = nullptr;
		int err = getaddrinfo(p_strAddress, nullptr, &hints, &result);
		if (err != 0 || !result)
		{
			throw std::runtime_error("Failed to resolve host: " + std::string(p_strAddress));
		}

		// Extract IPv4 address
		sockaddr_in* sockAddr = reinterpret_cast<sockaddr_in*>(result->ai_addr);
		Ip ip = sockAddr->sin_addr.S_un.S_addr;

		freeaddrinfo(result);

		return ip;
	}