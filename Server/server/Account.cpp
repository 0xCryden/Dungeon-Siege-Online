#include "Account.hpp"

Account::Account(uint32_t accountId,
	const std::string& username,
	const std::string& password,
	uint32_t admin)
	: m_id(accountId)
	, m_username(username)
	, m_password(password)
	, m_admin(admin)
{
}

void Account :: AddCharacter (Go * go)
{
	if (go != NULL)
	{
		m_objects.push_back (go);
	}
}

void Account :: RemoveCharacter (int slot)
{
	m_objects.erase(m_objects.begin() + (slot-1));
}

string Account :: Username () const
{
	return m_username;
}

string Account :: Password () const
{
	return m_password;
}

uint32_t Account :: Id () const
{
	return m_id;
}
