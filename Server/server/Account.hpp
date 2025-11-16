#ifndef ACCOUNT_HPP_
#define ACCOUNT_HPP_

	#include "../go/GoDb.hpp"
	#include "../net/Connection.hpp"
	
	#include <vector>
	using std::vector;
	
	class Account
	{
		public:
			
			Account (xmlNode * node);
			
			void AddCharacter(Go * go);
			void RemoveCharacter(int slot);

			string Username () const;
			string Password () const;
			uint32_t Id () const;
			vector<Go *> GetCharacters ()
			{
				return m_objects;
			}
			Go * GetGoFromSlot (int slot)
			{
				return m_objects.at (slot-1);
			}
			// <temp>
			Go * GetGo (int num)
			{
				return m_objects.at (num);
			}
			// </temp>
			
		private:
			
			string m_username;
			string m_password;
			uint32_t m_id;
			
			vector<Go *> m_objects;
	};

#endif /* ACCOUNT_HPP_ */
