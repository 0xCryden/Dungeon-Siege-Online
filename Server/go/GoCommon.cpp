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
#include "../Common.h"

#include "Go.hpp"
#include "GoCommon.hpp"
#include "../helper/Helper.h"

GoCommon :: GoCommon (Go * go) : GoComponent (go)
{
}

GoCommon::GoCommon(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
	if (tmplComp == nullptr)
		return;

	const string* f;
	if (f = tmplComp->GetField("auto_expiration_class")) { try { m_auto_expiration_class = *f; } catch (...) { m_auto_expiration_class = ""; } }
	if (f = tmplComp->GetField("forced_expiration_class")) { try { m_forced_expiration_class = *f; } catch (...) { m_forced_expiration_class = ""; } }
	if (f = tmplComp->GetField("screen_name")) { try { m_screen_name = StripQuotes(*f); } catch (...) { m_screen_name = ""; } }
}

GoCommon::GoCommon(Go* go, const std::map<std::string, std::string>& r) : GoComponent(go)
{
	m_auto_expiration_class = r.at("auto_expiration_class");
	m_forced_expiration_class = r.at("forced_expiration_class");
	m_screen_name = r.at("screen_name");
}

void GoCommon::Save(MySQL& db)
{
	std::string query =
		"INSERT INTO t_go_common "
		"(go_id, auto_expiration_class, forced_expiration_class, screen_name) VALUES ("
		+ std::to_string(GetGo()->Goid()) + ", '"
		+ m_auto_expiration_class + "', '"
		+ m_forced_expiration_class + "', '"
		+ m_screen_name + "') "
		"ON DUPLICATE KEY UPDATE "
		"auto_expiration_class = VALUES(auto_expiration_class), "
		"forced_expiration_class = VALUES(forced_expiration_class), "
		"screen_name = VALUES(screen_name)";

	db.AsyncQuery(query, [](const auto&) {});
}

string GoCommon :: AutoExpirationClass () const
{
	return m_auto_expiration_class;
}

string GoCommon :: ForcedExpirationClass () const
{
	return m_forced_expiration_class;
}
			
string GoCommon :: ScreenName () const
{
	return m_screen_name;
}

void GoCommon :: SetScreenName (const string & name)
{
	m_screen_name = name;
}
