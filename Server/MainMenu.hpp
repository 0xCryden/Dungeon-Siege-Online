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

#ifndef MAINMENU_HPP_
#define MAINMENU_HPP_

#include "server/Server.hpp"
#include "InGame.hpp"
#include "scripts/Player.hpp"
#include "Engine.hpp"

class MainMenu : public WorldState
{
	public:

		Account * rememberedAccount = NULL;

		MainMenu (Connection & connection) : WorldState (&connection)
		{
			//cout << "main menu entered" << endl;
		}

		~MainMenu ()
		{
		}

		void OnEnter ()
		{
		}

		void OnExit ()
		{
		}

		void Handle (const char * buffer)
		{
			Packet incoming (buffer);
			
			uint8_t type = incoming.ReadUInt8();
			switch (type)
			{
				case RSAUTHENTICATION:
				{
					string username = incoming.ReadString();
					string password = incoming.ReadString();

					Log::Write(Log::Level::INFO, "[AUTH] Request: " + username + " | " + password, true);

					Account * account = server.GetAccount (username);
					if (account != NULL)
					{
						rememberedAccount = account;
						bool authenticated = password == account->Password();

						Packet outgoing;
						outgoing.WriteUInt8 (RCAUTHENTICATION);
						outgoing.WriteUInt8 (authenticated == true ? 0 : 1);
						m_connection->Send (outgoing.Data(), outgoing.Size());
						
						if (authenticated == true)
						{
							Log::Write(Log::Level::INFO, "[AUTH] Login success: " + account->Username() + " | ID: " + to_string(account->Id()), true);
						}
					}
					else
					{
						Packet outgoing;
						outgoing.WriteUInt8 (RCAUTHENTICATION);
						outgoing.WriteUInt8 (1);
						m_connection->Send (outgoing.Data(), outgoing.Size());
					}
				}
				break;

				case RSREGISTER:
				{
					string username = incoming.ReadString();
					string password = incoming.ReadString();

					Log::Write(Log::Level::INFO, "[AUTH] ReqRegistration: " + username + " | " + password, true);

					Account * account = server.GetAccount (username);
					if (account != NULL)
					{
						Packet outgoing;
						outgoing.WriteUInt8 (RCREGISTER);
						outgoing.WriteUInt8 (1); // error code 1 username already exists
						m_connection->Send (outgoing.Data(), outgoing.Size());
						return;
					}

					if (containsInvalidChars(username) || containsInvalidChars(password)) // regex to username and pw
					{
						Packet outgoing;
						outgoing.WriteUInt8 (RCREGISTER);
						outgoing.WriteUInt8 (2); // error code 2 invalid characters used
						m_connection->Send (outgoing.Data(), outgoing.Size());
						return;
					}

					server.CreateAccount(username, password);

					Packet outgoing;
					outgoing.WriteUInt8 (RCREGISTER);
					outgoing.WriteUInt8 (0);
					m_connection->Send (outgoing.Data(), outgoing.Size());

				}
				break;

				case RSREQCHARLIST:
				{
					//cout << "RSREQCHARLIST" << endl;
					Account * account = rememberedAccount;
					if (account == NULL)
					{
						cout << "RSREQCHARLIST account error 1" << endl;
						Packet outgoing;
						outgoing.WriteUInt8 (RSREQCHARLIST);
						outgoing.WriteUInt8 (1); // error code 1 username already exists
						m_connection->Send (outgoing.Data(), outgoing.Size());
						return;
					}

					Packet outgoing;
					outgoing.WriteUInt8 (RSREQCHARLIST);
					outgoing.WriteUInt8 (0);
					vector<Go *> chars = account->GetCharacters();

					outgoing.WriteUInt8 (static_cast<uint8_t>(chars.size()));
					// TODO send character list
					for (Go* character : chars)  // use the getter
					{
						cout << "RSREQCHARLIST sending char " << character->Common()->ScreenName() << endl;
					    if (character)
					    {
					    	outgoing.WriteString(character->Common()->ScreenName());
					    	outgoing.WriteString(character->GetTitle());

					    	string statString =
					    	    "Str:" + to_string(static_cast<int>(floor(character->Actor()->GetSkillLevel("strength"))) + 10) +
					    	    " Dex:" + to_string(static_cast<int>(floor(character->Actor()->GetSkillLevel("dexterity"))) + 10) +
					    	    " Int:" + to_string(static_cast<int>(floor(character->Actor()->GetSkillLevel("intelligence"))) + 10);

					    	outgoing.WriteString(statString);

					    	outgoing.WriteString(to_string(static_cast<int>(floor(character->Actor()->GetSkillLevel("melee")))));
					    	outgoing.WriteString(to_string(static_cast<int>(floor(character->Actor()->GetSkillLevel("ranged")))));
					    	outgoing.WriteString(to_string(static_cast<int>(floor(character->Actor()->GetSkillLevel("nature magic")))));
					    	outgoing.WriteString(to_string(static_cast<int>(floor(character->Actor()->GetSkillLevel("combat magic")))));

					    }
					}
					m_connection->Send (outgoing.Data(), outgoing.Size());

				}
				break;

				case RSENTERGAME:
				{
					try {
						cout << "[DEBUG] RSENTERGAME start" << endl;

						int selectSlot = incoming.ReadUInt8();
						cout << "[DEBUG] Read selectSlot: " << selectSlot << endl;

						Account* account = rememberedAccount;

						Log::Write(Log::Level::INFO, "[AUTH] Request: " + account->Username() + " | " + account->Password() + " | selected charslot: " + to_string(selectSlot), true);

						if (account == NULL)
						{
							cout << "[ERROR] rememberedAccount is NULL" << endl;

							Packet outgoing;
							outgoing.WriteUInt8(RCENTERGAME);
							outgoing.WriteUInt8(1); // error code 1: username not found
							m_connection->Send(outgoing.Data(), outgoing.Size());
							return;
						}

						cout << "[DEBUG] Account username: " << account->Username() << endl;

						cout << "[DEBUG] Attempting to get Go from slot: " << selectSlot << endl;
						if (account->GetCharacters().size() == 0 || selectSlot < 1 || selectSlot > 4)
						{
							cout << "[ERROR] Account has no characters" << endl;

							Packet outgoing;
							outgoing.WriteUInt8(RCENTERGAME);
							outgoing.WriteUInt8(1); // error code 1: username not found
							m_connection->Send(outgoing.Data(), outgoing.Size());
							return;
						}

						Go* go = account->GetGoFromSlot(selectSlot);

						if (go)
							cout << "[DEBUG] Got Go from slot. ID: " << go->Goid() << endl;
						else
							cout << "[DEBUG] No Go found for slot: " << selectSlot << endl;

						if (go == NULL)
						{
							cout << "[ERROR] Null Go returned for valid login request" << endl;
							return;
						}
						if (g_engine.IsPlayer(go))
						{
							Log::Write(Log::Level::WARNING, "[AUTH] Char select Failed: " + account->Username() + " already connected | ID: " + to_string(go->Goid()), true);

							Packet outgoing;
							outgoing.WriteUInt8 (RCENTERGAME);
							outgoing.WriteUInt8 (2);
							m_connection->Send (outgoing.Data(), outgoing.Size());
							return;
						}

						Packet outgoing;
						outgoing.WriteUInt8(RCENTERGAME);
						outgoing.WriteUInt8(0); // success
						m_connection->Send(outgoing.Data(), outgoing.Size());

						Log::Write(Log::Level::INFO, "[AUTH] Char select success: " + account->Username() + " | ID: " + to_string(go->Goid()), true);

						go->AddComponent(new Player(m_connection, go));
						m_connection->SetWorldState(new InGame(*m_connection, go));
						g_engine.RegisterPlayer(go);

						cout << "[DEBUG] Player object created and registered for ID: " << go->Goid() << endl;
					}
					catch (const exception& e)
					{
						cout << "[EXCEPTION] RSENTERGAME: " << e.what() << endl;
					}
					catch (...)
					{
						cout << "[EXCEPTION] RSENTERGAME: Unknown crash" << endl;
					}
				}
				break;

				case RSDELETECHAR:
				{
					int selectSlot = incoming.ReadUInt8();
					Account* account = rememberedAccount;
					server.DeleteChar(selectSlot, account->Username(), account->Password());

					Packet outgoing;
					outgoing.WriteUInt8(RCDELETECHAR);
					outgoing.WriteUInt8(0); // success
					m_connection->Send(outgoing.Data(), outgoing.Size());
				}
				break;

				case RSCREATECHAR:
				{
					string charName = incoming.ReadString();
					int charType = incoming.ReadUInt8();
					int charHead = incoming.ReadUInt8();
					int charSkin = incoming.ReadUInt8();
					int charHair = incoming.ReadUInt8();
					int charShirt = incoming.ReadUInt8();
					int charPants = incoming.ReadUInt8();

					Account* account = rememberedAccount;
					if (charName == "")
					{
						Packet outgoing;
						outgoing.WriteUInt8(RCCREATECHAR);
						outgoing.WriteUInt8(2); // error all slots full
						m_connection->Send(outgoing.Data(), outgoing.Size());
						return;
					}
					else if (account->GetCharacters().size() < (size_t)4)
					{
						server.CreateChar(account->Username(), charName, charType, charHead, charSkin, charHair, charShirt, charPants);

						Packet outgoing;
						outgoing.WriteUInt8(RCCREATECHAR);
						outgoing.WriteUInt8(0); // success
						m_connection->Send(outgoing.Data(), outgoing.Size());
						return;
					}
					else
					{
						Packet outgoing;
						outgoing.WriteUInt8(RCCREATECHAR);
						outgoing.WriteUInt8(1); // error all slots full
						m_connection->Send(outgoing.Data(), outgoing.Size());
						return;
					}
				}
				break;

				default:
				{
				}
				break;
			} /* switch (type) */
		}

	protected:
		private: 
			bool containsInvalidChars(const string& str) {
			const string disallowed = R"(<>:/\|?*.%;)";
			return str.find_first_of(disallowed) != string::npos;
		}
		// string m_username;
		// string m_password;
};

#endif /* MAINMENU_HPP_ */
