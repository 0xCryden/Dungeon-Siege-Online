#ifndef SERVER_HPP_
#define SERVER_HPP_

#include "Account.hpp"
#include <random>
#include "../net/Network.hpp"
#include "../platform/windows/WinSockApi.h"
#include "../xml/XmlCleanupGuard.h"
#include "../mysql/MySQL.h"
class Engine;

class Server
{
	public:
		Server(/*Config config*/);
		~Server();

		void Start();
		void Loop();
		void Stop();

		Account* GetAccount(const string& account);
		void CreateAccount (const string & username, const string & password, std::function<void()> onInserted);

	private:
		void InitRng();
		void InitSystemGuards();
		void InitNetwork(const uint16_t port = 4000);
		void InitDatabase(const string& ip = "127.0.0.1", const string& username = "root", const string& password = "root", const string& database = "db_data");
		void InitTimers();

		void LoadResources(const string& folderName = "data");
		void LoadAccounts();

		// Config m_config;
		mt19937 m_rng;

		std::unique_ptr<WinSockApi> m_wsa;
		std::unique_ptr<XmlCleanupGuard> m_xml;
		Network m_network;

		map<string, Account*> m_accounts;
};
#endif /* SERVER_HPP_ */
