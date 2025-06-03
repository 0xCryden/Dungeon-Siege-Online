#ifndef SERVER_HPP_
#define SERVER_HPP_

	#include "Account.hpp"
	
	#include <map>
	using std::map;
	
	class Server
	{
		public:
			
			~Server ();
			
			void LoadAccounts (const string & filename);
			
			Account * GetAccount (const string & account);

			void CreateAccount (const string & username, const string & password);
			void DeleteChar (int selectSlot, const string & username, const string & password);
			void CreateChar (const string & username, const string & charName, int charType, int charHead, int charSkin, int charHair, int charShirt, int charPants);

		private:
			
			map<string, Account *> m_accounts;
	};
	
	extern Server server;

#endif /* SERVER_HPP_ */
