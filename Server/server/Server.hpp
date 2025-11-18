#ifndef SERVER_HPP_
#define SERVER_HPP_

#include "Account.hpp"
#include <random>
#include "../net/Network.hpp"
#include "../platform/windows/WinSockApi.h"
#include "../xml/XmlCleanupGuard.h"
class Engine;

class Server
{
	public:
		Server(/*Config config*/);
		~Server();

		void Start();
		void Loop();
		void Stop();

		void LoadAccounts (const string & filename);
		Account * GetAccount (const string & account);
		void CreateAccount (const string & username, const string & password);
		void DeleteChar (int selectSlot, const string & username, const string & password);
		void CreateChar (const string & username, const string & charName, int charType, int charHead, int charSkin, int charHair, int charShirt, int charPants);

	private:
		void InitRng();
		void InitSystemGuards();
		void InitNetwork(const uint16_t port = 4000);
		void LoadResources(const string& folderName = "data");
		void InitTimers();

		// Config m_config;
		mt19937 m_rng;

		std::unique_ptr<WinSockApi> m_wsa;
		std::unique_ptr<XmlCleanupGuard> m_xml;
		//Engine* m_engine;
		//std::unique_ptr<Engine> m_engine;
		Network m_network;

		map<string, Account*> m_accounts;
};
#endif /* SERVER_HPP_ */
