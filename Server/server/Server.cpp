
#include "Server.hpp"

#include "../Engine.hpp"
#include "../helper/Helper.h"
#include "../events/SendWorldMessageEvent.hpp"
#include "../Globals.h"

Server :: Server(/*Config config*/)
	: m_wsa(std::make_unique<WinSockApi>())
	, m_network(*this)
{
}

Server :: ~Server ()
{
	for (auto& [name, acc] : m_accounts)
		delete acc;
}

void Server::Start()
{
	Log::Write(Log::Level::INFO, "", true);
	Log::Write(Log::Level::INFO, "    ###   DungeonSiegeOnline v0.5   ###", true);
	Log::Write(Log::Level::INFO, "", true);

	InitRng();
	InitSystemGuards();
	InitNetwork(/*m_config.port*/);
	InitDatabase(/*m_config.ip*//*, m_config.db_username*//*, m_config.db_password*//*, m_config.db_name*/);
	LoadResources();
	InitTimers();

	Log::Write(Log::Level::INFO, "Server startup complete.");
}

void Server::Loop()
{
	while (g_engine.IsRunning()) {
		m_network.Listen();
		g_engine.Loop();
		sleep_microseconds(TICK_MICROSECONDS);
	}
}

void Server::Stop()
{
	g_engine.Db().Close();
	Log::Write(Log::Level::INFO, "Stopping server...");
}

void Server::InitRng()
{
	std::random_device rd;
	m_rng = mt19937(rd());
	Log::Write(Log::Level::INFO, "RNG initialized");
}

void Server::InitSystemGuards()
{
	m_wsa = std::make_unique<WinSockApi>();
	m_xml = std::make_unique<XmlCleanupGuard>();
	Log::Write(Log::Level::INFO, "System guards initialized");
}

void Server::InitNetwork(const uint16_t port)
{
	m_network.Bind(port);

	Log::Write(Log::Level::INFO, "Network bound on port " + to_string(port), true);
}

void Server::InitDatabase(const string& ip, const string& username, const string& password, const string& database)
{
	if (!g_engine.Db().Connect("127.0.0.1", "root", "root", "db_data")) {
		std::cerr << "DB connection failed" << std::endl;
		return;
	}

	Log::Write(Log::Level::INFO, "DB connected on ip " + ip, true);
}

void Server::LoadResources(const string& dataDir)
{
	Log::Write(Log::Level::INFO, "Loading maps ...");
	g_world.LoadAllMaps();

	Log::Write(Log::Level::INFO, "Loading templates ...");
	gas.LoadTemplates();
	gas.LoadMapTemplates();

	Log::Write(Log::Level::INFO, "Loading GO database ...");
	godb.LoadContentDb();
	godb.LoadSqlDb();

	Log::Write(Log::Level::INFO, "Loading accounts...");
	LoadAccounts();

	Log::Write(Log::Level::INFO, "Instantiating map templates...");
	godb.InstantiateMapTemplates();

	Log::Write(Log::Level::INFO, "Resource loading complete");
}

void Server::InitTimers()
{
	int64_t now = CurrentTime();

	int64_t delay_second = SECOND - (now % SECOND);
	int64_t delay_minute = MINUTE - (now % MINUTE);
	int64_t delay_hour = HOUR - (now % HOUR);

	if (delay_second == 0) delay_second = SECOND;
	if (delay_minute == 0) delay_minute = MINUTE;
	if (delay_hour == 0) delay_hour = HOUR;

	PostWorldMessage(we_timer_second, 0, 0, "", delay_second);
	PostWorldMessage(we_timer_minute, 0, 0, "", delay_minute);
	PostWorldMessage(we_timer_hour, 0, 0, "", delay_hour);
	// PostWorldMessage(we_timer_day, 0, 0, "", DAY);
	// PostWorldMessage(we_timer_week, 0, 0, "", WEEK);
}



void Server::LoadAccounts()
{
	m_accounts = g_engine.Db().LoadAccounts();
}

Account * Server :: GetAccount (const string & account)
{
	map<string, Account *>::iterator iterator = m_accounts.find (account);
	
	return (iterator != m_accounts.end() ? iterator->second : NULL);
}

void Server::CreateAccount(const std::string& username, const std::string& password, std::function<void()> onInserted)
{
	g_engine.Db().InsertAccount(username, password, [this, username, password, onInserted = std::move(onInserted)]()
		{
			Account* acc = new Account(
				m_accounts.size() + 1,
				username,
				password,
				0
			);

			m_accounts[username] = acc;

			if (onInserted)
				onInserted();
		});
}