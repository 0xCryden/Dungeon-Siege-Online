#include "../Common.h"

#include "Go.hpp"
#include "GoBody.hpp"

GoBody :: GoBody (Go * go) : GoComponent (go)
{
}

GoBody::GoBody(Go* go, const TemplateComponent* tmplComp) : GoComponent(go)
{
	if (tmplComp == nullptr)
		return;

	const string* f;

	if (f = tmplComp->GetField("avg_move_velocity")) { try { m_avg_move_velocity = std::stof(*f); } catch (...) { m_avg_move_velocity = 0.0f; } }
	if (f = tmplComp->GetField("max_move_velocity")) { try { m_max_move_velocity = std::stof(*f); } catch (...) { m_max_move_velocity = 0.0f; } }
	if (f = tmplComp->GetField("min_move_velocity")) { try { m_min_move_velocity = std::stof(*f); } catch (...) { m_min_move_velocity = 0.0f; } }
}
GoBody::GoBody(Go* go, const std::map<std::string, std::string>& r) : GoComponent(go)
{
	m_avg_move_velocity = std::stof(r.at("avg_move_velocity"));
	m_max_move_velocity = std::stof(r.at("max_move_velocity"));
	m_min_move_velocity = std::stof(r.at("min_move_velocity"));
}

void GoBody::Save(MySQL& db)
{
	std::string q =
		"INSERT INTO t_go_body (go_id, avg_move_velocity, max_move_velocity, min_move_velocity) VALUES (" +
		std::to_string(GetGo()->Goid()) + ", " +
		std::to_string(m_avg_move_velocity) + ", " +
		std::to_string(m_max_move_velocity) + ", " +
		std::to_string(m_min_move_velocity) + ") "
		"ON DUPLICATE KEY UPDATE "
		"avg_move_velocity=VALUES(avg_move_velocity), "
		"max_move_velocity=VALUES(max_move_velocity), "
		"min_move_velocity=VALUES(min_move_velocity)";

	db.AsyncQuery(q, [](const auto&) {});
}

float GoBody :: AvgMoveVelocity () const
{
	return m_avg_move_velocity;
}

float GoBody :: MaxMoveVelocity () const
{
	return m_max_move_velocity;
}

float GoBody :: MinMoveVelocity () const
{
	return m_min_move_velocity;
}

void GoBody :: SetAvgMoveVelocity (float velocity)
{
	m_avg_move_velocity = velocity;
}
