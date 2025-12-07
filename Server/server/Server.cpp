
#include "Server.hpp"

#include "../Engine.hpp"
#include "../helper/Helper.h"
#include "../events/SendWorldMessageEvent.hpp"
#include "../Globals.h"

std::vector<std::string> modelStrings = {
		"humanboy",
		"humangirl",
		"dso_dwarf",
		"dso_skeleton",
		"dso_utraean_boy",
		"dso_utraean_girl",
		"dso_halfgiant"
};

std::vector<std::vector<std::string>> headStrings = {
    // humanboy (portal_farmboy)
    {
        "default", "m_c_gah_fb_hlmt_head-02", "m_c_gah_fb_hlmt_head-03", "m_c_gah_fb_hlmt_head-04", "m_c_gah_fb_hlmt_head-05"
    },
    // humangirl (portal_farmgirl)
    {
        "default",  "m_c_gah_fg_hlmt_head-02", "m_c_gah_fg_hlmt_head-03", "m_c_gah_fg_hlmt_head-04", "m_c_gah_fg_hlmt_head-05"
    },
    // dwarf (portal_dwarf)
    {
        "default"
    },
    // skeleton (portal_skeleton)
    {
        "default"
    },
    // utraean_boy (portal_utraean_boy)
    {
        "default", "m_c_gah_fb_hlmt_head-02", "m_c_gah_fb_hlmt_head-03", "m_c_gah_fb_hlmt_head-04", "m_c_gah_fb_hlmt_head-05"
    },
    // utraean_girl (portal_utraean_girl)
    {
        "default", "m_c_gah_fg_hlmt_head-02", "m_c_gah_fg_hlmt_head-03", "m_c_gah_fg_hlmt_head-04", "m_c_gah_fg_hlmt_head-05"
    },
    // halfgiant (portal_halfgiant)
    {
        "default"
    }
};

std::vector<std::vector<std::string>> skinStrings = {
    // humanboy
    {
        "b_c_gah_fb_skin_001", "b_c_gah_fb_skin_002", "b_c_gah_fb_skin_003", "b_c_gah_fb_skin_004",
        "b_c_gah_fb_skin_005", "b_c_gah_fb_skin_006", "b_c_gah_fb_skin_007", "b_c_gah_fb_skin_008",
        "b_c_gah_fb_skin_009", "b_c_gah_fb_skin_010", "b_c_gan_fb_skin_jondar"
    },
    // humangirl
    {
        "b_c_gah_fg_skin_001", "b_c_gah_fg_skin_002", "b_c_gah_fg_skin_003", "b_c_gah_fg_skin_004",
        "b_c_gah_fg_skin_005", "b_c_gah_fg_skin_006", "b_c_gah_fg_skin_007", "b_c_gah_fg_skin_008",
        "b_c_gah_fg_skin_009", "b_c_gah_fg_skin_010", "b_c_gan_fg_skin_lyssa"
    },
    // dwarf
    {
        "b_c_gan_df_skin_01", "b_c_gan_df_skin_02", "b_c_gan_df_skin_03",
        "b_c_gan_df_skin_04", "b_c_gan_df_skin_05", "b_c_gan_df_skin_06"
    },
    // skeleton
    {
        "b_c_ecm_sk", "b_c_ecm_sk-02", "b_c_ecm_skg"
    },
    // utraean_boy
    {
        "b_c_gan_utraean_skin_01", "b_c_gan_utraean_skin_02", "b_c_gan_utraean_skin_03"
    },
    // utraean_girl
    {
        "b_c_gan_utraean_skin_04", "b_c_gan_utraean_skin_06"
    },
    // halftgiant
    {
        "b_c_gan_hg_skin_01", "b_c_gan_hg_skin_02", "b_c_gan_hg_skin_03",
        "b_c_gan_hg_skin_04", "b_c_gan_hg_skin_05", "b_c_gan_hg_skin_06",
        "b_c_gan_hg_skin_07"
    }
};

std::vector<std::vector<std::string>> hairStrings = {
    // humanboy
    {
        "b_c_gah_fb_hair_001", "b_c_gah_fb_hair_002", "b_c_gah_fb_hair_003", "b_c_gah_fb_hair_004",
        "b_c_gah_fb_hair_005", "b_c_gah_fb_hair_006", "b_c_gah_fb_hair_007", "b_c_gah_fb_hair_008",
        "b_c_gah_fb_hair_009", "b_c_gah_fb_hair_010"
    },
    // humangirl
    {
        "b_c_gah_fg_hair_001", "b_c_gah_fg_hair_002", "b_c_gah_fg_hair_003", "b_c_gah_fg_hair_004",
        "b_c_gah_fg_hair_005", "b_c_gah_fg_hair_006", "b_c_gah_fg_hair_007", "b_c_gah_fg_hair_008",
        "b_c_gah_fg_hair_009", "b_c_gah_fg_hair_010"
    },
    // dwarf
    {
        "default"// empty
    },
    // skeleton
    {
        "default"// empty
    },
    // utraean_boy
    {
        "b_c_gah_fb_hair_001", "b_c_gah_fb_hair_002", "b_c_gah_fb_hair_003", "b_c_gah_fb_hair_004",
        "b_c_gah_fb_hair_005", "b_c_gah_fb_hair_006", "b_c_gah_fb_hair_007", "b_c_gah_fb_hair_008",
        "b_c_gah_fb_hair_009", "b_c_gah_fb_hair_010"
    },
    // utraean_girl
    {
        "default",
        "b_c_gah_fg_hair_001", "b_c_gah_fg_hair_002", "b_c_gah_fg_hair_003", "b_c_gah_fg_hair_004",
        "b_c_gah_fg_hair_005", "b_c_gah_fg_hair_006", "b_c_gah_fg_hair_007", "b_c_gah_fg_hair_008",
        "b_c_gah_fg_hair_009", "b_c_gah_fg_hair_010"
    },
    // halftgiant
    {
        "default",
        "b_c_gah_fb_hair_001", "b_c_gah_fb_hair_002", "b_c_gah_fb_hair_003", "b_c_gah_fb_hair_004",
        "b_c_gah_fb_hair_005", "b_c_gah_fb_hair_006", "b_c_gah_fb_hair_007", "b_c_gah_fb_hair_008",
        "b_c_gah_fb_hair_009", "b_c_gah_fb_hair_010"
    }
};

std::vector<std::vector<std::string>> shirtStrings = {
    // humanboy
    {
    	"b_c_pos_a1_jondar",
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    },
    // humangirl
    {
    	"b_c_pos_a1_lyssa",
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    },
    // dwarf
    {
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    },
    // skeleton
    {
        "default"// empty
    },
    // utraean_boy
    {
        "b_c_pos_a1_utraean-01",
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    },
    // utraean_girl
    {
        "b_c_pos_a1_utraean-01", "b_c_pos_a1_utraean-04",
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    },
    // halftgiant
    {
        "b_c_pos_a1_100", "b_c_pos_a1_200", "b_c_pos_a1_300",
        "b_c_pos_a1_shrt_004", "b_c_pos_a1_shrt_005", "b_c_pos_a1_shrt_008",
        "b_c_pos_a1_shrt_009", "b_c_pos_a1_shrt_010", "b_c_pos_a1_shrt_011", "b_c_pos_a1_shrt_012",
        "b_c_pos_a1_shrt_013", "b_c_pos_a1_shrt_014", "b_c_pos_a1_shrt_015", "b_c_pos_a1_shrt_016",
        "b_c_pos_a1_shrt_017", "b_c_pos_a1_shrt_018"
    }
};

std::vector<std::vector<std::string>> pantsStrings = {
    // humanboy
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    },
    // humangirl
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    },
    // dwarf
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    },
    // skeleton
    {
        "default" // empty
    },
    // utraean_boy
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    },
    // utraean_girl
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    },
    // halftgiant
    {
        "b_c_pos_a1_pant_004", "b_c_pos_a1_pant_005", "b_c_pos_a1_pant_006", "b_c_pos_a1_pant_008",
        "b_c_pos_a1_pant_009", "b_c_pos_a1_pant_010", "b_c_pos_a1_pant_011", "b_c_pos_a1_pant_012",
        "b_c_pos_a1_pant_013", "b_c_pos_a1_pant_014", "b_c_pos_a1_pant_015", "b_c_pos_a1_pant_016"
    }
};



Server :: Server(/*Config config*/)
	: m_wsa(std::make_unique<WinSockApi>())
	, m_network(*this)
{
	//m_engine = std::make_unique<Engine>();
	/*
	m_config = config;
	m_member = config.member;
	*/
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
	InitNetwork(); // InitNetwork(m_config.port);
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

void Server::LoadResources(const string& dataDir)
{
	Log::Write(Log::Level::INFO, "Loading maps...");
	g_world.LoadAllMaps();

	Log::Write(Log::Level::INFO, "Loading templates...");
	gas.LoadTemplates();
	gas.LoadMapTemplates();
	//godb.LoadContentDb();

	Log::Write(Log::Level::INFO, "Loading GO database...");
	godb.LoadGoDbFolder("items");
	godb.LoadGoDbFolder("actors");
	//godb.LoadContentDb(dataDir + "\\static\\actors.xml");

	Log::Write(Log::Level::INFO, "Loading accounts...");
	LoadAccounts(dataDir + "\\dynamic\\accounts.xml");

	Log::Write(Log::Level::INFO, "Instantiating map templates...");
	godb.InstantiateMapTemplates();
	//godb.LoadSpawns();

	Log::Write(Log::Level::INFO, "Resource loading complete");
}


void Server::InitTimers()
{
	PostWorldMessage(we_frustum_active_state_changed, 0, 0, "", CALC_FRUSTUM_DELAY);

	PostWorldMessage(we_timer_second, 0, 0, "", SECOND);
	PostWorldMessage(we_timer_minute, 0, 0, "", MINUTE);
	PostWorldMessage(we_timer_hour, 0, 0, "", HOUR);
	//PostWorldMessage(we_timer_day, 0, 0, "", DAY);
	//PostWorldMessage(we_timer_week, 0, 0, "", WEEK);
}

void Server :: LoadAccounts (const string & filename)
{
	xmlDoc * document = xmlReadFile (filename.c_str(), NULL, 0);
	if (document == NULL)
	{
		return; // throw
	}
	
	xmlNode * root = xmlDocGetRootElement (document);
	if (root == NULL)
	{
		xmlFree (document);
		return; // throw
	}
	
	xmlNode * node = NULL;
	for (node = root->children; node != NULL; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE) continue;
		
		if (xmlStrEqual (node->name, (const xmlChar *) "account") != 0)
		{
			string username = xml::ReadAttribute<string> (node, "username", "");
			if (username.empty() != true)
			{
				Account * account = new Account (node);
				m_accounts[username] = account;
				std::cout << "Loaded account " << username.c_str() << std::endl;
			}
		}
	}
	
	xmlFreeDoc (document);
}

Account * Server :: GetAccount (const string & account)
{
	map<string, Account *>::iterator iterator = m_accounts.find (account);
	
	return (iterator != m_accounts.end() ? iterator->second : NULL);
}

void Server :: CreateAccount (const string & username, const string & password)
{
	const std::string path = "data/dynamic/accounts.xml";

	xmlDoc * document = xmlReadFile(path.c_str(), NULL, 0);
	if (document == NULL)
	{
		cout << "Error accounts.xml not found" << endl;
		return;
	}

	xmlNode * root = xmlDocGetRootElement(document);
	if (root == NULL)
	{
		xmlFreeDoc(document);
		return;
	}

	// --- Step 1: Determine next ID ---
	int maxId = 0;
	for (xmlNode * node = root->children; node != NULL; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && xmlStrEqual(node->name, BAD_CAST "account"))
		{
			int id = xml::ReadAttribute<int>(node, "id", 0);
			if (id > maxId)
				maxId = id;
		}
	}
	int nextId = maxId + 1;

	// --- Step 2: Create properly formed <account> with opening/closing tags ---
	xmlNode * newAccountNode = xmlNewNode(NULL, BAD_CAST "account");
	xmlAddChild(root, newAccountNode); // ensures <account> ... </account>

	xml::SetAttribute(newAccountNode, "username", username);
	xml::SetAttribute(newAccountNode, "password", password);
	xml::SetAttribute(newAccountNode, "id", nextId);

	// --- Step 3: Save formatted XML ---
	if (!xml::SaveFile(document, path))
	{
		cout << "Error: Failed to save accounts.xml" << endl;
		xmlFreeDoc(document);
		return;
	}

	// --- Step 4: Register in memory ---
	Account * account = new Account(newAccountNode);
	m_accounts[username] = account;
	std::cout << "Created and loaded account " << username << std::endl;

	xmlFreeDoc(document);
}


void Server :: DeleteChar (int selectSlot, const string & username, const string & password)
{
	const std::string path = "data/dynamic/accounts.xml";

	xmlDoc * document = xmlReadFile (path.c_str(), NULL, 0);
	if (document == NULL)
	{
		cout << "Error accounts.xml not found" << endl;
		return; // throw
	}
	xmlNode * root = xmlDocGetRootElement (document);
	if (root == NULL)
	{
		xmlFree (document);
		return; // throw
	}

	bool deleted = false;

	// Iterate <account> nodes
	for (xmlNode* account = root->children; account; account = account->next)
	{
		if (account->type != XML_ELEMENT_NODE || xmlStrcmp(account->name, BAD_CAST "account") != 0)
			continue;

		string user = xml::XReadString(account, "username", "");
		string pass = xml::XReadString(account, "password", "");
		if (user != username || pass != password)
			continue;

		// Valid account found, look for N-th <go> child (1-based)
		int slot = 0;
		for (xmlNode* child = account->children; child; child = child->next)
		{
			if (child->type != XML_ELEMENT_NODE || xmlStrcmp(child->name, BAD_CAST "go") != 0)
				continue;

			slot++;
			if (slot == selectSlot)
			{
				xmlUnlinkNode(child);
				xmlFreeNode(child);
				deleted = true;
				break;
			}
		}
		break; // done after matching account
	}

	if (!deleted)
	{
		cout << "Character slot not found or credentials invalid\n";
		xmlFreeDoc(document);
		return;
	}

	if (!xml::SaveFile(document, path))
	{
		cout << "Error: Failed to save accounts.xml\n";
		xmlFreeDoc(document);
		return;
	}

	std::cout << "Deleted character in slot " << selectSlot << " for user " << username << std::endl;

	xmlFreeDoc(document);  // Always OK to leave here

	Account* acc = GetAccount(username);
	if (!acc)
	{
		std::cout << "Account not found for username: " << username << std::endl;
		return;
	}

	const auto& chars = acc->GetCharacters();
	std::cout << "Account has " << chars.size() << " characters" << std::endl;

	if (selectSlot < 1 || selectSlot > static_cast<int>(chars.size()))
	{
		std::cout << "Invalid slot: " << selectSlot << " (out of range)" << std::endl;
		return;
	}

	int index = selectSlot - 1; // convert 1-based to 0-based
	Go* removed = chars[index];

	acc->RemoveCharacter(selectSlot);//GetCharacters().erase(acc->GetCharacters().begin() + index);

	std::cout << "Removed Go at slot " << selectSlot
			  << " (Goid=" << removed->Goid() << ")" << std::endl;

}

void Server :: CreateChar (const string & username, const string & charName, int charType, int charHead, int charSkin, int charHair, int charShirt, int charPants)
{
	std::cout << "Customization: "
	          << "type=" << charType
	          << " head=" << charHead
	          << " skin=" << charSkin << "/" << skinStrings[charType].size()
	          << " hair=" << charHair << "/" << hairStrings[charType].size()
	          << " pants=" << charPants << "/" << pantsStrings[charType].size()
	          << " shirt=" << charShirt << "/" << shirtStrings[charType].size()
	          << std::endl;

	if (charType < 0 || charType >= static_cast<int>(modelStrings.size()) ||
	    charHead  < 0 || charHead  >= static_cast<int>(headStrings[charType].size()) ||
	    charSkin  < 0 || charSkin  >= static_cast<int>(skinStrings[charType].size()) ||
	    charHair  < 0 || charHair  >= static_cast<int>(hairStrings[charType].size()) ||
	    charPants < 0 || charPants >= static_cast<int>(pantsStrings[charType].size()) ||
	    charShirt < 0 || charShirt >= static_cast<int>(shirtStrings[charType].size()))
	{
	    std::cout << "Invalid character customization index." << std::endl;
	    return;
	}


	string strModel = modelStrings[charType];//"humangirl";
	// Get selected character parts based on charType index
	string strHead  = headStrings[charType][charHead];
	string strSkin  = skinStrings[charType][charSkin];
	string strHair  = hairStrings[charType][charHair];
	string strPants = pantsStrings[charType][charPants];
	string strShirt = shirtStrings[charType][charShirt];

	// Log the selected strings for debugging
	std::cout << "Character model: " << strModel << std::endl;
	std::cout << "Selected Head:   " << strHead << std::endl;
	std::cout << "Selected Skin:   " << strSkin << std::endl;
	std::cout << "Selected Hair:   " << strHair << std::endl;
	std::cout << "Selected Pants:  " << strPants << std::endl;
	std::cout << "Selected Shirt:  " << strShirt << std::endl;

	size_t count = g_engine.GetPlayerCharacters().size();
	string charFile = "data\\dynamic\\actors\\" + to_string(count+1) + ".xml";

	ofstream file(charFile);
	if (!file.is_open())
	{
		cerr << "Failed to create char XML file: " << charFile << endl;
		return;
	}
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	file << "<objects>\n";
	file << "	<go id=\"" + to_string(count+1) + "\">\n";

	file << "		<actor>\n";
	file << "			<alignment value=\"aa_good\"/>\n";
	file << "			<can_level_up value=\"1\"/>\n";
	file << "			<skills>\n";
	file << "				<skill name=\"uber\" level=\"0\" experience=\"0\"/>\n";
	file << "				<skill name=\"strength\" level=\"0\" experience=\"0\"/>\n";
	file << "				<skill name=\"intelligence\" level=\"0\" experience=\"0\"/>\n";
	file << "				<skill name=\"dexterity\" level=\"0\" experience=\"0\"/>\n";
	file << "				<skill name=\"melee\" level=\"0\" experience=\"0\"/>\n";
	file << "				<skill name=\"ranged\" level=\"0\" experience=\"0\"/>\n";
	file << "				<skill name=\"nature magic\" level=\"0\" experience=\"0\"/>\n";
	file << "				<skill name=\"combat magic\" level=\"0\" experience=\"0\"/>\n";
	file << "			</skills>\n";
	file << "		</actor>\n";

	file << "		<aspect>\n";
	file << "			<bounding_sphere_radius value=\"1\"/>\n";
	file << "			<current_life value=\"49\"/>\n";
	file << "			<current_mana value=\"30\"/>\n";
	file << "			<flesh value=\"" + strSkin;
	//if (strHair != "default") {
	file << "," + strHair; //}
	file <<	"\"/>\n";
	file << "			<cloth value=\"" + strPants + "," + strShirt + "\"/>\n";
	file << "			<is_invincible value=\"0\"/>\n";
	file << "			<is_visible value=\"1\"/>\n";
	file << "			<life_recovery_period value=\"4.0\"/>\n";
	file << "			<life_recovery_unit value=\"1.0\"/>\n";
	file << "			<mana_recovery_period value=\"3.0\"/>\n";
	file << "			<mana_recovery_unit value=\"1.0\"/>\n";
	file << "			<life_state value=\"ls_alive_conscious\"/>\n";
	file << "			<last_died value=\"0\"/>\n";
	file << "			<max_life value=\"49\"/>\n";
	file << "			<max_mana value=\"30\"/>\n";
	file << "			<model value=\"" + strModel + "\"/>\n";
	file << "			<render_scale value=\"1.0\"/>\n";
	file << "		</aspect>\n";

	file << "		<attack>\n";
	file << "			<damage_max value=\"0\"/>\n";
	file << "			<damage_min value=\"0\"/>\n";
	file << "			<attack_range value=\"0\"/>\n";
	file << "			<critical_hit_chance value=\"0\"/>\n";
	file << "			<is_two_handed value=\"0\"/>\n";
	file << "			<reload_delay value=\"1.0\"/>\n";
	file << "		</attack>\n";

	file << "		<body>\n";
	file << "			<avg_move_velocity value=\"4.5\"/>\n";
	file << "			<max_move_velocity value=\"4.5\"/>\n";
	file << "			<min_move_velocity value=\"4.5\"/>\n";
	file << "		</body>\n";

	file << "		<common>\n";
	file << "			<auto_expiration_class value=\"never\"/>\n";
	file << "			<forced_expiration_class value=\"auto\"/>\n";
	file << "			<screen_name value=\"" << charName << "\"/>\n";
	file << "		</common>\n";

	file << "		<defend/>\n";
	file << "		<mind/>\n";
	file << "		<inventory>\n";
	//if (strHead != "default") {
	file << "			<custom_head value=\"" << strHead << "\"/>\n"; //}
	file << "		</inventory>\n";

	file << "		<placement region=\"town_center\">\n";
	file << "			<position node=\"1323345966\" x=\"0\" y=\"0\" z=\"0\"/>\n";
	file << "		</placement>\n";

	file << "	</go>\n";
	file << "</objects>\n";
	file.close();

	godb.LoadGoDbSingleChar((uint32_t)(count+1));

	const std::string path = "data/dynamic/accounts.xml";

	xmlDoc * document = xmlReadFile (path.c_str(), NULL, 0);
	if (document == NULL)
	{
		cout << "Error accounts.xml not found" << endl;
		return; // throw
	}
	xmlNode * root = xmlDocGetRootElement (document);
	if (root == NULL)
	{
		xmlFree (document);
		return; // throw
	}

	// Find matching account node
	xmlNode* accountNode = nullptr;
	for (xmlNode* node = root->children; node != nullptr; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && xmlStrEqual(node->name, BAD_CAST "account"))
		{
			string uname = xml::ReadAttribute<string>(node, "username", "");
			if (uname == username)
			{
				accountNode = node;
				break;
			}
		}
	}

	if (!accountNode)
	{
		cout << "Account not found in XML for username: " << username << endl;
		xmlFreeDoc(document);
		return;
	}
	// Add new <go> entry
	xmlNode* goNode = xmlNewChild(accountNode, NULL, BAD_CAST "go", NULL);
	xml::SetAttribute(goNode, "id", (uint32_t)(count+1));

	if (!xml::SaveFile(document, path))
	{
		cout << "Error: Failed to save accounts.xml" << endl;
		xmlFreeDoc(document);
		return;
	}
	xmlFreeDoc(document);

	// --- Step 2: Register in-memory character to account ---
	Account* acc = GetAccount(username);
	if (!acc)
	{
		std::cout << "Account not found in memory: " << username << std::endl;
		return;
	}

	Go * go = godb.FindGoById ((uint32_t)(count+1));
	if (!go)
	{
		std::cout << "Error: Go not found by id after loading: " << count + 1 << std::endl;
		return;
	}
	acc->AddCharacter(go);

	std::cout << "Created character \"" << charName << "\" with Goid=" << go->Goid() << " for account " << username << std::endl;
}
