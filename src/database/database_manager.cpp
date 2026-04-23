#include "database/database_manager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h> // 需要OpenSSL用于密码哈希

namespace Sanguosha {
namespace Database {

DatabaseManager::DatabaseManager() : db_(nullptr), initialized_(false) {}

DatabaseManager::~DatabaseManager() {
    if (db_) {
        sqlite3_close(db_);
    }
}

DatabaseManager& DatabaseManager::Instance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initialize(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return true;
    }
    
    int rc = sqlite3_open(dbPath.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    if (!createTables()) {
        std::cerr << "Failed to create database tables" << std::endl;
        return false;
    }
    
    initialized_ = true;
    std::cout << "Database initialized successfully" << std::endl;
    return true;
}

bool DatabaseManager::createTables() {
    const char* createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            email TEXT UNIQUE NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_login DATETIME
        );
    )";
    
    return executeQuery(createUsersTable);
}

bool DatabaseManager::registerUser(const std::string& username, const std::string& password, 
                                  const std::string& email, uint32_t& userId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return false;
    }
    
    // 检查用户是否已存在
    if (userExists(username)) {
        return false;
    }
    
    std::string passwordHash = hashPassword(password);
    
    std::stringstream query;
    query << "INSERT INTO users (username, password_hash, email) VALUES ('"
          << username << "', '" << passwordHash << "', '" << email << "');";
    
    if (!executeQuery(query.str())) {
        return false;
    }
    
    // 获取新创建的用户ID
    sqlite3_int64 rowId = sqlite3_last_insert_rowid(db_);
    userId = static_cast<uint32_t>(rowId);
    
    return true;
}

bool DatabaseManager::authenticateUser(const std::string& username, const std::string& password, uint32_t& userId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return false;
    }
    
    std::string passwordHash = hashPassword(password);
    
    std::stringstream query;
    query << "SELECT id FROM users WHERE username = '" << username 
          << "' AND password_hash = '" << passwordHash << "' LIMIT 1;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, query.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        userId = static_cast<uint32_t>(sqlite3_column_int(stmt, 0));
        success = true;
        
        // 更新最后登录时间
        std::stringstream updateQuery;
        updateQuery << "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = " << userId << ";";
        executeQuery(updateQuery.str());
    }
    
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::userExists(const std::string& username) {
    std::stringstream query;
    query << "SELECT COUNT(*) FROM users WHERE username = '" << username << "' LIMIT 1;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, query.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }
    
    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }
    
    sqlite3_finalize(stmt);
    return exists;
}

std::string DatabaseManager::hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool DatabaseManager::executeQuery(const std::string& query) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

} // namespace Database
} // namespace Sanguosha