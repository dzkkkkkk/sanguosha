# Database module CMakeLists.txt
add_library(database OBJECT
    database_manager.cpp
)

target_include_directories(database
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${MYSQL_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(database
    PRIVATE
        ${MYSQL_LIBRARIES}
        OpenSSL::SSL
        OpenSSL::Crypto
)