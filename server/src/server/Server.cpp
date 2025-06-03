
#include "Server.hpp"

#include "../Engine.hpp"
Server server;

Server :: ~Server ()
{
	map<string, Account *>::iterator iterator = m_accounts.begin();
	while (iterator != m_accounts.end())
	{
		delete iterator->second;
		iterator++;
	}
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
	// Create region XML file path
	size_t count = engine.GetPlayerCharacters().size();

	// TODO skins
	string strSkin = "b_c_gah_fg_skin_003";
	string strHair = "b_c_gah_fg_hair_002";
	string strPants = "b_c_pos_a1_pant_005";
	string strShirt = "b_c_pos_a1_shrt_008";

	string strModel = "humangirl";
	switch (charType)
	{
		case 0: // humanboy
		{
			strModel = "humanboy";
		}
		break;

		case 1: // humangirl
		{
			strModel = "humangirl";
		}
		break;

		case 2: // dwarf
		{
			strModel = "m_c_gan_df_pos_a1";
		}
		break;

		case 3: // skeleton
		{
			strModel = "m_c_ecm_sk_pos_a0";
		}
		break;

		case 4: // utraeanboy
		{
			strModel = "humanboy";
		}
		break;

		case 5: // utraeangirl
		{
			strModel = "humangirl";
		}
		break;

		case 6: // halfgiant
		{
			strModel = "m_c_gan_hg_pos_a1";
		}
		break;

		default: break;
	}

	string charFile = "data\\dynamic\\actors\\" + to_string(count+1) + ".xml";
	// Create the XML file for the region
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
	file << "			<flesh value=\"" + strSkin + "," + strHair + "\"/>\n";
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
	file << "		<inventory/>\n";

	file << "		<placement region=\"town_center\">\n";
	file << "			<position node=\"1323345966\" x=\"0\" y=\"0\" z=\"0\"/>\n";
	file << "		</placement>\n";

	file << "	</go>\n";
	file << "</objects>\n";
	file.close();

	godb.LoadGoDbSingleChar((u_int32_t)(count+1));

	// TODO add char to accounts.xml
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
	xml::SetAttribute(goNode, "id", (u_int32_t)(count+1));

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

	Go * go = godb.FindGoById ((u_int32_t)(count+1));
	if (!go)
	{
		std::cout << "Error: Go not found by id after loading: " << count + 1 << std::endl;
		return;
	}
	acc->AddCharacter(go);

	std::cout << "Created character \"" << charName << "\" with Goid=" << go->Goid() << " for account " << username << std::endl;
}
