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

bool DatabaseManager::initialize(const std::string& dbName,
                                 const std::string& host,
                                 const std::string& user,
                                 const std::string& password,
                                 unsigned int port) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return true;
    }
    
    int rc = sqlite3_open(dbName.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open SQLite database: " << sqlite3_errmsg(db_) << std::endl;
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
    const std::string createUsersTable = R"(
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
    
    if (userExists(username)) {
        return false;
    }
    
    std::string passwordHash = hashPassword(password);
    
    std::stringstream query;
    query << "INSERT INTO users (username, password_hash, email) VALUES ('"
          << escapeString(username) << "', '" << passwordHash << "', '" << escapeString(email) << "');";
    
    if (!executeQuery(query.str())) {
        return false;
    }
    
    userId = static_cast<uint32_t>(sqlite3_last_insert_rowid(db_));
    return true;
}

bool DatabaseManager::authenticateUser(const std::string& username, const std::string& password, 
                                      uint32_t& userId) {
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
        std::cerr << "Failed to prepare query: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_ROW);
    if (success) {
        userId = static_cast<uint32_t>(sqlite3_column_int(stmt, 0));
    }
    
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::userExists(const std::string& username) {
    // Note: This function assumes the caller has already acquired the mutex
    // to avoid deadlocks when called from registerUser
    
    if (!initialized_) {
        return false;
    }
    
    std::stringstream query;
    query << "SELECT COUNT(*) FROM users WHERE username = '" << escapeString(username) << "' LIMIT 1;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, query.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare query: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }
    
    rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0);
    
    sqlite3_finalize(stmt);
    return exists;
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

std::string DatabaseManager::escapeString(const std::string& value) {
    // SQLite doesn't need manual escaping like MySQL, but we'll implement basic escaping
    std::string escaped = value;
    // Replace single quotes with double single quotes
    size_t pos = 0;
    while ((pos = escaped.find("'", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "''");
        pos += 2;
    }
    return escaped;
}

std::string DatabaseManager::hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

} // namespace Database
} // namespace Sanguosha