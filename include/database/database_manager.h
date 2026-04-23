#pragma once

#include <sqlite3.h>
#include <string>
#include <memory>
#include <mutex>

namespace Sanguosha {
namespace Database {

class DatabaseManager {
public:
    static DatabaseManager& Instance();
    
    bool initialize(const std::string& dbPath = "sanguosha.db");
    bool registerUser(const std::string& username, const std::string& password, const std::string& email, uint32_t& userId);
    bool authenticateUser(const std::string& username, const std::string& password, uint32_t& userId);
    bool userExists(const std::string& username);
    
private:
    DatabaseManager();
    ~DatabaseManager();
    
    bool createTables();
    std::string hashPassword(const std::string& password);
    bool executeQuery(const std::string& query);
    
    sqlite3* db_;
    std::mutex mutex_;
    bool initialized_;
};

} // namespace Database
} // namespace Sanguosha