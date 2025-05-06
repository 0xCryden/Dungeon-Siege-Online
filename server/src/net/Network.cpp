#include "../MainMenu.hpp"
#include "Network.hpp"
#include <winsock2.h>

Network::Network()
{
    std::cout << "[INIT] Network constructor entered" << std::endl;

#ifdef _WIN32
    int result = WSAStartup(MAKEWORD(2, 2), &m_wsadata);
    std::cout << "[INIT] WSAStartup result: " << result << std::endl;

    if (result != 0) {
        std::cerr << "[ERROR] WSAStartup failed with error " << result << std::endl;
        exit(3);  // or throw std::runtime_error
    }
#endif

    FD_ZERO(&m_descriptors);
    FD_ZERO(&m_active);

    std::cout << "[INIT] Network constructor finished" << std::endl;
}

Network::~Network()
{
    std::cout << "[CLEANUP] Cleaning up network..." << std::endl;

    m_listening.Close();

    for (auto& conn : m_connections) {
        conn.Close();
    }

#ifdef _WIN32
    WSACleanup();
#endif

    std::cout << "[CLEANUP] Network cleanup complete." << std::endl;
}

void Network::Bind(u_int16_t port)
{
    m_listening.Bind(port);
    m_listening.Listen();

    std::cout << "[BIND] Listening on port " << port << ", socket: " << m_listening.data() << std::endl;

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

    timeval tv = {0, 0};

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

                    std::cout << "[ACCEPT] New connection accepted: socket " << conn.data() << std::endl;
                }
                catch (int& err) {
                    if (err != EWOULDBLOCK) {
                        std::cerr << "[ERROR] Exception while accepting connection: " << err << std::endl;
                        throw;
                    }
                }
            } else {
                std::cerr << "[ACCEPT] Connection refused: server full." << std::endl;
            }
        }
    }
}

void Network::Receive()
{
    int activity = 0;

    if (!m_sockets.empty()) {
        timeval tv = {0, 0};
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
                } catch (int& e) {
                    std::cerr << "[ERROR] Receive failed on socket " << sock << ", error: " << e << std::endl;

                    m_sockets.erase(sock);
                    FD_CLR(sock, &m_descriptors);
                    conn->Close();
                    m_connections.erase(conn);

                    std::cout << "[CLOSE] Connection closed: socket " << sock << std::endl;
                }

                if (--activity <= 0)
                    break;
            }
        }
    } else if (activity == 0) {
        // No socket activity
    } else {
        std::cerr << "[ERROR] select() failed with error: " << WSAGetLastError() << std::endl;
    }
}
