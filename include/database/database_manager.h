#pragma once

#include <mysql/mysql.h>
#include <string>
#include <mutex>

namespace Sanguosha {
namespace Database {

class DatabaseManager {
public:
    static DatabaseManager& Instance();
    
    bool initialize(const std::string& dbName = "sanguosha",
                    const std::string& host = "127.0.0.1",
                    const std::string& user = "root",
                    const std::string& password = "",
                    unsigned int port = 3306);
    bool registerUser(const std::string& username, const std::string& password, const std::string& email, uint32_t& userId);
    bool authenticateUser(const std::string& username, const std::string& password, uint32_t& userId);
    bool userExists(const std::string& username);
    
private:
    DatabaseManager();
    ~DatabaseManager();
    
    bool createTables();
    std::string hashPassword(const std::string& password);
    bool executeQuery(const std::string& query);
    std::string escapeString(const std::string& value);
    
    MYSQL* conn_;
    std::mutex mutex_;
    bool initialized_;
};

} // namespace Database
} // namespace Sanguosha