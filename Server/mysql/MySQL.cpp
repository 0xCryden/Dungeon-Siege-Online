#include "MySQL.h"
#include "../server/Account.hpp"
#include "../Engine.hpp"

MySQL::MySQL(size_t numThreads)
    : numThreads_(numThreads), stop_(false)
{
    workers_.resize(numThreads_);
    connections_.resize(numThreads_, nullptr);
}

MySQL::~MySQL() {
    Close();
}

bool MySQL::Connect(const std::string& host,
    const std::string& user,
    const std::string& password,
    const std::string& db,
    unsigned int port)
{
    // Create sync (main-thread-only) connection
    syncConnection_ = mysql_init(nullptr);
    if (!mysql_real_connect(
        syncConnection_,
        host.c_str(), user.c_str(), password.c_str(),
        db.c_str(), port, nullptr, CLIENT_MULTI_STATEMENTS))
    {
        std::cerr << "Sync MySQL connect failed: "
            << mysql_error(syncConnection_) << std::endl;
        return false;
    }

    mysql_set_character_set(syncConnection_, "utf8mb4");

    for (size_t i = 0; i < numThreads_; ++i) {
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "mysql_init failed for worker " << i << std::endl;
            return false;
        }

        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
            db.c_str(), port, nullptr, CLIENT_MULTI_STATEMENTS))
        {
            std::cerr << "MySQL connect failed for worker " << i
                << ": " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return false;
        }

        // Set UTF8
        if (mysql_set_character_set(conn, "utf8mb4") != 0) {
            std::cerr << "Failed to set utf8mb4: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return false;
        }

        connections_[i] = conn;
        workers_[i] = std::thread(&MySQL::WorkerThread, this, i);
    }
    return true;
}

void MySQL::Close() {
    stop_ = true;
    queueCV_.notify_all();

    for (auto& t : workers_) if (t.joinable()) t.join();

    for (auto* conn : connections_) {
        if (conn) mysql_close(conn);
    }

    if (syncConnection_) mysql_close(syncConnection_);

    connections_.clear();
}

void MySQL::InsertAccount(const string& username, const string& password, std::function<void()> onInserted)
{
    // Build the INSERT query
    std::string query = "INSERT INTO t_accounts (username, password_plain) VALUES ('" +
        EscapeString(username) + "', '" +
        EscapeString(password) + "')";

    // Fire async query
    AsyncQuery(query, [onInserted = std::move(onInserted)](QueryResult&&)
        {
            // The INSERT is done on the DB thread, now push to main thread
            g_engine.m_mainThreadJobs.push([onInserted = std::move(onInserted)]()
                {
                    if (onInserted)
                        onInserted();
                });
        });
}

void MySQL::InsertGo(
    uint32_t goid,
    const std::string& charName,
    const std::string& charType,
    const std::string& charHead,
    const std::string& charSkin,
    const std::string& charHair,
    const std::string& charShirt,
    const std::string& charPants,
    Account* account,
    std::function<void()> onInserted)
{
    std::string escCharName = EscapeString(charName);
    std::string escModel = EscapeString(charType);
    std::string escHead = EscapeString(charHead);
    std::string escSkin = EscapeString(charSkin);
    std::string escHair = EscapeString(charHair);
    std::string escShirt = EscapeString(charShirt);
    std::string escPants = EscapeString(charPants);

    auto pending = std::make_shared<std::atomic<int>>(1); // initial t_gos insert

    auto done = [pending, onInserted]()
        {
            if (pending->fetch_sub(1) == 1 && onInserted)
            {
                g_engine.m_mainThreadJobs.push([onInserted]() { onInserted(); });
            }
        };

    // Step 1: Insert into t_gos
    std::string insertGoQuery =
        "INSERT INTO t_gos (go_id, template_name, pcontent_query) VALUES (" +
        std::to_string(goid) + ", '" + escModel + "', '');";

    AsyncQuery(insertGoQuery,
        [this, goid, escCharName, escModel, escHead, escSkin, escHair, escShirt, escPants, account, pending, done](const auto&)
        {
            // Step 2: Insert t_go_components
            std::vector<std::string> components = { "actor", "aspect", "common", "inventory", "placement", "attack", "body", "defend", "gui", "magic", "mind" };
            pending->fetch_add(static_cast<int>(components.size()));

            for (const auto& comp : components)
            {
                std::string compQuery =
                    "INSERT INTO t_go_components (go_id, component_type) VALUES (" +
                    std::to_string(goid) + ", '" + EscapeString(comp) + "');";

                AsyncQuery(compQuery, [done](const auto&) { done(); });
            }

            // Step 3: Insert component data
            pending->fetch_add(11); // one per component insert function

            InsertGoActor(goid, [done]() { done(); });
            InsertGoAspect(goid, escSkin, escHair, escPants, escShirt, escModel, [done]() { done(); });
            InsertGoCommon(goid, escCharName, [done]() { done(); });
            InsertGoInventory(goid, escHead, [done]() { done(); });
            InsertGoPlacement(goid, "", [done]() { done(); });
            InsertGoAttack(goid, [done]() { done(); });
            InsertGoBody(goid, [done]() { done(); });
            InsertGoDefend(goid, [done]() { done(); });
            InsertGoGui(goid, [done]() { done(); });
            InsertGoMagic(goid, [done]() { done(); });
            InsertGoMind(goid, [done]() { done(); });

            // Step 4: Update first free char slot in t_accounts
            // Step 1: Query account row
            std::string selectQuery = "SELECT char_id_1, char_id_2, char_id_3, char_id_4, char_id_5, char_id_6, char_id_7, char_id_8 "
                "FROM t_accounts WHERE account_id = " + std::to_string(account->Id()) + " LIMIT 1;";

            AsyncQuery(selectQuery, [this, goid, account, done](const auto& rows)
                {
                    if (rows.empty()) { done(); return; }

                    const auto& row = rows[0];
                    std::string slotColumn;

                    for (int i = 1; i <= 8; ++i)
                    {
                        std::string col = "char_id_" + std::to_string(i);
                        if (row.at(col) == "0" || row.at(col).empty())
                        {
                            slotColumn = col;
                            break;
                        }
                    }

                    if (slotColumn.empty()) { done(); return; } // no free slot

                    std::string updateQuery = "UPDATE t_accounts SET " + slotColumn + " = " + std::to_string(goid) +
                        " WHERE account_id = " + std::to_string(account->Id()) + ";";

                    AsyncQuery(updateQuery, [done](const auto&) { done(); });
                });

            // Step 5: decrement initial t_gos insert counter
            done();
        }
    );
}


void MySQL::InsertGoAttack(uint32_t goId, std::function<void()> cb)
{
    std::string query =
        "INSERT INTO t_go_attack (go_id, attack_range, critical_hit_chance, damage_min, damage_max, is_two_handed, reload_delay, attack_class) VALUES (" +
        std::to_string(goId) + ", 1.0, 0.0, 0.0, 0.0, 0, 0, 'ac_beastfu');";

    AsyncQuery(query, [cb](const auto&) { cb(); });
}

void MySQL::InsertGoBody(uint32_t goId, std::function<void()> cb)
{
    std::string query =
        "INSERT INTO t_go_body (go_id, avg_move_velocity, max_move_velocity, min_move_velocity) VALUES (" +
        std::to_string(goId) + ", 0.0, 0.0, 0.0);";

    AsyncQuery(query, [cb](const auto&) { cb(); });
}
void MySQL::InsertGoDefend(uint32_t goId, std::function<void()> cb)
{
    std::string query =
        "INSERT INTO t_go_defend (go_id, defense, defend_class) VALUES (" +
        std::to_string(goId) + ", 0.0, 'dc_skin');";

    AsyncQuery(query, [cb](const auto&) { cb(); });
}
void MySQL::InsertGoGui(uint32_t goId, std::function<void()> cb)
{
    std::string query =
        "INSERT INTO t_go_gui (go_id, equip_slot, inventory_width, inventory_height, equip_requirements) VALUES (" +
        std::to_string(goId) + ", 'es_none', 0, 0, '');";

    AsyncQuery(query, [cb](const auto&) { cb(); });
}
void MySQL::InsertGoMagic(uint32_t goId, std::function<void()> cb)
{
    std::string query =
        "INSERT INTO t_go_magic (go_id, spell_class, required_level, cast_sub_animation, cast_range, cast_reload_delay, effect_duration, is_defensive, is_offensive) VALUES (" +
        std::to_string(goId) + ", 'mc_none', 1.0, 1, 1.0, 1, 1, 0, 0);";

    AsyncQuery(query, [cb](const auto&) { cb(); });
}
void MySQL::InsertGoMind(uint32_t goId, std::function<void()> cb)
{
    std::string query =
        "INSERT INTO t_go_mind (go_id, melee_engage_range) VALUES (" +
        std::to_string(goId) + ", 0.0);";

    AsyncQuery(query, [cb](const auto&) { cb(); });
}


void MySQL::InsertGoActor(uint32_t goId, std::function<void()> cb)
{
    std::string query =
        "INSERT INTO t_go_actor (go_id, alignment, can_level_up, "
        "skill_uber, skill_uber_exp, "
        "skill_strength, skill_strength_exp, "
        "skill_intelligence, skill_intelligence_exp, "
        "skill_dexterity, skill_dexterity_exp, "
        "skill_melee, skill_melee_exp, "
        "skill_ranged, skill_ranged_exp, "
        "skill_nature_magic, skill_nature_magic_exp, "
        "skill_combat_magic, skill_combat_magic_exp) "
        "VALUES (" +
        std::to_string(goId) + ", 'aa_good', 1, "
        "0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0);";

    AsyncQuery(query, [cb](const auto&) { cb(); });
}

void MySQL::InsertGoAspect(uint32_t goId,
    const std::string& flesh,
    const std::string& hair,
    const std::string& pants,
    const std::string& shirt,
    const std::string& model,
    std::function<void()> cb)
{
    // Combine flesh parts (skin + hair)
    std::string fleshCombined = flesh;
    if (!hair.empty())
        fleshCombined += "," + hair;

    // Combine cloth parts (pants + shirt)
    std::string clothCombined = pants + "," + shirt;

    std::string query =
        "INSERT INTO t_go_aspect (go_id, bounding_sphere_radius, current_life, max_life, current_mana, max_mana, "
        "flesh, cloth, is_invincible, is_visible, life_state, last_died, model, render_scale, experience_value) VALUES (" +
        std::to_string(goId) + ", 1.0, 49, 49, 30, 30, '" +
        EscapeString(fleshCombined) + "', '" +
        EscapeString(clothCombined) + "', 0, 1, 'ls_alive_conscious', 0, '" +
        EscapeString(model) + "', 1.0, 0);";

    AsyncQuery(query, [cb](const auto&) { cb(); });
}

void MySQL::InsertGoCommon(uint32_t goId, const std::string& screenName, std::function<void()> cb)
{
    std::string query =
        "INSERT INTO t_go_common (go_id, auto_expiration_class, forced_expiration_class, screen_name) "
        "VALUES (" + std::to_string(goId) + ", 'never','auto','" + screenName + "');";
    AsyncQuery(query, [cb](const auto&) { cb(); });
}

void MySQL::InsertGoInventory(uint32_t goId, const std::string& head, std::function<void()> cb)
{
    std::string query =
        "INSERT INTO t_go_inventory (go_id, custom_head, selected_active_location) "
        "VALUES (" + std::to_string(goId) + ", '" + head + "',0);";
    AsyncQuery(query, [cb](const auto&) { cb(); });
}

void MySQL::InsertGoPlacement(uint32_t goId, const std::string& head, std::function<void()> cb)
{
    std::string query =
        "INSERT INTO t_go_placement (go_id, region, node_id, pos_x, pos_y, pos_z) "
        "VALUES (" + std::to_string(goId) + ", 'town_center', 1323345966, 0, 0, 0);";
    AsyncQuery(query, [cb](const auto&) { cb(); });
}

std::map<std::string, Account*> MySQL::LoadAccounts()
{
    std::map<std::string, Account*> accountsByUsername;
    std::map<int, Account*> accountsById;

    QueryResult rows;

    // Select all 8 character ID columns
    bool success = QuerySync(
        "SELECT account_id, username, password_plain, admin, "
        "char_id_1, char_id_2, char_id_3, char_id_4, "
        "char_id_5, char_id_6, char_id_7, char_id_8 "
        "FROM t_accounts "
        "ORDER BY account_id",
        rows
    );

    if (!success)
    {
        std::cerr << "Failed to load accounts from DB" << std::endl;
        return accountsByUsername;
    }

    for (const auto& row : rows)
    {
        int accountId = std::stoi(row.at("account_id"));
        const std::string& username = row.at("username");

        if (username.empty())
            continue;

        Account* account = nullptr;

        auto it = accountsById.find(accountId);
        if (it == accountsById.end())
        {
            // Create account once
            account = new Account(
                accountId,
                username,
                row.at("password_plain"),
                std::stoi(row.at("admin"))
            );

            accountsById[accountId] = account;
            accountsByUsername[username] = account;

            std::cout << "Loaded account " << username << std::endl;
        }
        else
        {
            account = it->second;
        }

        // Assign character IDs if they exist
        for (int i = 1; i <= 8; ++i)
        {
            auto itChar = row.find("char_id_" + std::to_string(i));
            if (itChar != row.end() && !itChar->second.empty())
            {
                uint32_t goId = static_cast<uint32_t>(std::stoul(itChar->second));
                Go* go = godb.FindGoById(goId);
                if (go != nullptr)
                {
                    account->AddCharacter(go);
                    go->SetAccount(account);
                }
            }
        }
    }

    return accountsByUsername;
}

void MySQL::AsyncQuery(string query, std::function<void(QueryResult&&)> callback)
{
    {
        std::lock_guard lock(queueMutex_);
        taskQueue_.push(Task{ std::move(query), std::move(callback) });
    }
    queueCV_.notify_one();
}

bool MySQL::QuerySync(
    const std::string& query,
    std::vector<std::map<std::string, std::string>>& out)
{
    if (syncConnection_ == nullptr)
    {
        std::cerr << "[MySQL] Sync Connection is null!" << std::endl;
        return false;
    }

    MYSQL* conn = syncConnection_;

    if (!conn)
    {
        std::cerr << "[MySQL] connection not initialized! Call Connect() first." << std::endl;
        return false;
    }

    if (mysql_query(conn, query.c_str()) != 0)
    {
        std::cerr << "[MySQL] Query failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res)
        return true; // e.g. INSERT or empty result

    int numFields = mysql_num_fields(res);
    MYSQL_FIELD* fields = mysql_fetch_fields(res);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)))
    {
        std::map<std::string, std::string> record;
        for (int i = 0; i < numFields; ++i)
            record[fields[i].name] = row[i] ? row[i] : "";
        out.push_back(std::move(record));
    }

    mysql_free_result(res);
    return true;
}

string MySQL::EscapeString(const std::string& input, size_t workerIndex) {
    if (workerIndex >= connections_.size() || !connections_[workerIndex]) return input;

    std::string escaped;
    escaped.resize(input.length() * 2 + 1);
    unsigned long len = mysql_real_escape_string(connections_[workerIndex], &escaped[0], input.c_str(), input.length());
    escaped.resize(len);
    return escaped;
}

void MySQL::WorkerThread(size_t index)
{
    MYSQL* conn = connections_[index];

    // DEBUG: log active database
    /*if (mysql_query(conn, "SELECT DATABASE()") == 0)
    {
        if (MYSQL_RES* res = mysql_store_result(conn))
        {
            if (MYSQL_ROW row = mysql_fetch_row(res))
            {
                std::cout
                    << "[Worker " << index << "] DATABASE() = "
                    << (row[0] ? row[0] : "NULL")
                    << std::endl;
            }
            mysql_free_result(res);
        }
    }*/

    while (!stop_)
    {
        Task task;
        {
            std::unique_lock lock(queueMutex_);
            queueCV_.wait(lock, [&] {
                return stop_ || !taskQueue_.empty();
                });

            if (stop_) return;

            task = std::move(taskQueue_.front());
            taskQueue_.pop();
        }

        QueryResult result;
        int rc = mysql_query(conn, task.query.c_str());
        if (rc != 0)
        {
            std::cerr << "MySQL query failed: " << mysql_error(conn) << std::endl;
        }
        else
        {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res)
            {
                int numFields = mysql_num_fields(res);
                MYSQL_FIELD* fields = mysql_fetch_fields(res);

                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)))
                {
                    std::map<std::string, std::string> record;
                    for (int i = 0; i < numFields; ++i)
                        record[fields[i].name] = row[i] ? row[i] : "";
                    result.push_back(std::move(record));
                }
                mysql_free_result(res);
            }
        }

        task.onComplete(std::move(result));
    }
}


