#pragma once

#include "../Common.h"
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <iostream>
#include "../go/Go.hpp"

class Account;

using QueryResult = std::vector<std::map<std::string, std::string>>;

class MySQL {
public:
    using QueryCallback = function<void(const vector<map<string, string>>&)>;

    MySQL(size_t numThreads = 4);
    ~MySQL();

    bool Connect(const string& host,
        const string& user,
        const string& password,
        const string& db,
        unsigned int port = 3306);
    void Close();

    std::map<std::string, Account*> LoadAccounts();
    void InsertAccount(const string& username, const string& password, std::function<void()> onInserted);

    void InsertGo(
        uint32_t goid,
        const string& charName,
        const string& charType,
        const string& charHead,
        const string& charSkin,
        const string& charHair,
        const string& charShirt,
        const string& charPants,
        Account* account,
        std::function<void()> onInserted);

    void AsyncQuery(string query, std::function<void(QueryResult&&)> callback);
    bool QuerySync(const std::string& query, std::vector<std::map<std::string, std::string>>& out);

private:

    struct Task {
        string query;
        std::function<void(QueryResult&&)> onComplete;
    };
    void InsertGoAttack(uint32_t goId, std::function<void()> cb);
    void InsertGoBody(uint32_t goId, std::function<void()> cb);
    void InsertGoDefend(uint32_t goId, std::function<void()> cb);
    void InsertGoGui(uint32_t goId, std::function<void()> cb);
    void InsertGoMagic(uint32_t goId, std::function<void()> cb);
    void InsertGoMind(uint32_t goId, std::function<void()> cb);
    void InsertGoActor(uint32_t goId, std::function<void()> cb);
    void InsertGoAspect(uint32_t goId,
        const std::string& flesh, const std::string& hair,
        const std::string& pants, const std::string& shirt,
        const std::string& model,
        std::function<void()> cb);
    void InsertGoCommon(uint32_t goId, const std::string& screenName, std::function<void()> cb);
    void InsertGoInventory(uint32_t goId, const std::string& head, std::function<void()> cb);
    void InsertGoPlacement(uint32_t goId, const std::string& head, std::function<void()> cb);

    void WorkerThread(size_t index);
    // Escape string safely
    string EscapeString(const string& input, size_t workerIndex = 0);

    size_t numThreads_;
    vector<thread> workers_;
    vector<MYSQL*> connections_; 
    MYSQL* syncConnection_ = nullptr;

    queue<Task> taskQueue_;
    mutex queueMutex_;
    condition_variable queueCV_;
    atomic<bool> stop_;
};
