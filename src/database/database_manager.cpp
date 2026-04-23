#include "database/database_manager.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h> // 需要OpenSSL用于密码哈希

namespace Sanguosha {
namespace Database {

DatabaseManager::DatabaseManager() : conn_(nullptr), initialized_(false) {}

DatabaseManager::~DatabaseManager() {
    if (conn_) {
        mysql_close(conn_);
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
    
    conn_ = mysql_init(nullptr);
    if (!conn_) {
        std::cerr << "MySQL initialization failed" << std::endl;
        return false;
    }
    
    if (!mysql_real_connect(conn_, host.c_str(), user.c_str(), password.c_str(), nullptr, port, nullptr, 0)) {
        std::cerr << "Cannot connect to MySQL: " << mysql_error(conn_) << std::endl;
        mysql_close(conn_);
        conn_ = nullptr;
        return false;
    }

    std::string createDbQuery = "CREATE DATABASE IF NOT EXISTS `" + dbName + "` CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;";
    if (!executeQuery(createDbQuery)) {
        return false;
    }

    if (mysql_select_db(conn_, dbName.c_str()) != 0) {
        std::cerr << "Failed to select database: " << mysql_error(conn_) << std::endl;
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
            id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
            username VARCHAR(128) UNIQUE NOT NULL,
            password_hash VARCHAR(128) NOT NULL,
            email VARCHAR(256) UNIQUE NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_login DATETIME
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
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
    
    std::string safeUsername = escapeString(username);
    std::string safeEmail = escapeString(email);
    std::string passwordHash = hashPassword(password);
    std::string safePasswordHash = escapeString(passwordHash);
    
    std::stringstream query;
    query << "INSERT INTO users (username, password_hash, email) VALUES ('"
          << safeUsername << "', '" << safePasswordHash << "', '" << safeEmail << "');";
    
    if (!executeQuery(query.str())) {
        return false;
    }
    
    userId = static_cast<uint32_t>(mysql_insert_id(conn_));
    return true;
}

bool DatabaseManager::authenticateUser(const std::string& username, const std::string& password, uint32_t& userId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return false;
    }
    
    std::string safeUsername = escapeString(username);
    std::string safePasswordHash = escapeString(hashPassword(password));
    
    std::stringstream query;
    query << "SELECT id FROM users WHERE username = '" << safeUsername 
          << "' AND password_hash = '" << safePasswordHash << "' LIMIT 1;";
    
    if (mysql_query(conn_, query.str().c_str()) != 0) {
        std::cerr << "Failed to execute query: " << mysql_error(conn_) << std::endl;
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn_);
    if (!result) {
        std::cerr << "Failed to store query result: " << mysql_error(conn_) << std::endl;
        return false;
    }
    
    bool success = false;
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
        userId = static_cast<uint32_t>(std::stoul(row[0]));
        success = true;

        std::stringstream updateQuery;
        updateQuery << "UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = " << userId << ";";
        executeQuery(updateQuery.str());
    }
    
    mysql_free_result(result);
    return success;
}

bool DatabaseManager::userExists(const std::string& username) {
    std::string safeUsername = escapeString(username);
    std::stringstream query;
    query << "SELECT COUNT(*) FROM users WHERE username = '" << safeUsername << "' LIMIT 1;";

    if (mysql_query(conn_, query.str().c_str()) != 0) {
        return false;
    }

    MYSQL_RES* result = mysql_store_result(conn_);
    if (!result) {
        return false;
    }

    bool exists = false;
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row && row[0]) {
        exists = std::stoul(row[0]) > 0;
    }

    mysql_free_result(result);
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

std::string DatabaseManager::escapeString(const std::string& value) {
    if (!conn_) {
        return value;
    }
    
    std::string escaped;
    escaped.resize(value.size() * 2 + 1);
    unsigned long length = mysql_real_escape_string(conn_, &escaped[0], value.c_str(), static_cast<unsigned long>(value.size()));
    escaped.resize(length);
    return escaped;
}

bool DatabaseManager::executeQuery(const std::string& query) {
    if (mysql_query(conn_, query.c_str()) != 0) {
        std::cerr << "SQL error: " << mysql_error(conn_) << std::endl;
        return false;
    }
    return true;
}

} // namespace Database
} // namespace Sanguosha