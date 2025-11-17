#include "../MainMenu.hpp"
#include "Network.hpp"
#include <winsock2.h>

#include <chrono>
#include <iomanip>

Network::Network()
{
    Log::Write(Log::Level::INFO, "[INIT] Network constructor entered", true);

#ifdef _WIN32
    int result = WSAStartup(MAKEWORD(2, 2), &m_wsadata);
    Log::WriteF(Log::Level::INFO, "[INIT] WSAStartup result: %d", result);

    if (result != 0) {
        Log::WriteF(Log::Level::ERR, "[ERROR] WSAStartup failed with error %d", result);
        exit(3);  // or throw std::runtime_error
    }
#endif

    FD_ZERO(&m_descriptors);
    FD_ZERO(&m_active);

    Log::Write(Log::Level::INFO, "[INIT] Network constructor finished", true);
}

Network::~Network()
{
    Log::Write(Log::Level::INFO, "[CLEANUP] Cleaning up network...", true);

    m_listening.Close();

    for (auto& conn : m_connections) {
        conn.Close();
    }

#ifdef _WIN32
    WSACleanup();
#endif

    Log::Write(Log::Level::INFO, "[CLEANUP] Network cleanup complete.", true);
}

void Network::Bind(uint16_t port)
{
    m_listening.Bind(port);
    m_listening.Listen();

    Log::WriteF(Log::Level::INFO, "[BIND] Listening on port %u, socket: %llu", port, static_cast<unsigned long long>(m_listening.data()));

    m_listening.SetNonBlockingFlag(true);
    FD_SET(m_listening.data(), &m_descriptors);
}

void Network::Listen()
{
    Accept();
    Receive();
}

void Network::Accept()
{
    fd_set fdsr;
    FD_ZERO(&fdsr);
    FD_SET(m_listening.data(), &fdsr);

    timeval tv = { 0, 0 };

    int activity = select(0, &fdsr, NULL, NULL, &tv);

    if (activity > 0 && FD_ISSET(m_listening.data(), &fdsr)) {
        SOCKET descriptor = m_listening.Accept();
        if (descriptor != INVALID_SOCKET) {
            if (FD_SETSIZE - m_sockets.size() > 0) {
                try {
                    Connection c(descriptor);
                    m_connections.push_back(c);

                    Connection& conn = *m_connections.rbegin();
                    conn.SetNonBlockingFlag(true);
                    conn.SetWorldState(new MainMenu(conn));

                    m_sockets.insert(conn.data());
                    FD_SET(conn.data(), &m_descriptors);

                    Log::WriteF(Log::Level::INFO, "[ACCEPT] New conn: socket %llu", static_cast<unsigned long long>(conn.data()));
                }
                catch (int& err) {
                    if (err != EWOULDBLOCK) {
                        Log::WriteF(Log::Level::ERR, "[ERROR] Exception while accepting connection: %d", err);
                        throw;
                    }
                }
            }
            else {
                Log::Write(Log::Level::WARNING, "[ACCEPT] Connection refused: server full.", true);
            }
        }
    }
}

void Network::Receive()
{
    int activity = 0;

    if (!m_sockets.empty()) {
        timeval tv = { 0, 0 };
        m_active = m_descriptors;

        activity = select(0, &m_active, NULL, NULL, &tv);
    }

    if (activity > 0) {

        auto it = m_connections.begin();
        while (it != m_connections.end()) {
            auto conn = it++;
            SOCKET sock = conn->data();

            if (FD_ISSET(sock, &m_active)) {
                try {
                    conn->Receive();
                }
                catch (int& e) {
                    //Log::WriteF(Log::Level::ERR, "[ERROR] Receive failed on socket %llu, error: %d", static_cast<unsigned long long>(sock), e);

                    m_sockets.erase(sock);
                    FD_CLR(sock, &m_descriptors);
                    conn->Close();
                    m_connections.erase(conn);

                    Log::WriteF(Log::Level::INFO, "[CLOSE] Connection closed: socket %llu, error: %d", static_cast<unsigned long long>(sock), e);
                }

                if (--activity <= 0)
                    break;
            }
        }
    }
    else if (activity == 0) {
        // No socket activity
    }
    else {
        Log::WriteF(Log::Level::ERR, "[ERROR] select() failed with error: %d", WSAGetLastError());
    }
}
