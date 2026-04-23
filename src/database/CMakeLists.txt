# Database module CMakeLists.txt
add_library(database OBJECT
    database_manager.cpp
)

target_include_directories(database
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${SQLite3_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(database
    PRIVATE
        SQLite::SQLite3
        OpenSSL::SSL
        OpenSSL::Crypto
)