cmake_minimum_required(VERSION 3.12)
project(SanguoshaServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置项目源码目录
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Boost
find_package(Boost 1.71 REQUIRED COMPONENTS system)

# Protobuf
find_package(Protobuf REQUIRED)

# SQLite3
find_package(SQLite3 REQUIRED)

# OpenSSL
find_package(OpenSSL REQUIRED)

# 包含目录
include_directories(
    include 
    ${Boost_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

# 添加核心模块
add_subdirectory(src/game)
add_subdirectory(src/network)
add_subdirectory(src/room)
add_subdirectory(src/database)

# 简化可执行文件
add_executable(sanguosha_server
    src/main.cpp
    include/sanguosha.pb.cc  # 直接引用手动生成的protobuf源文件
    $<TARGET_OBJECTS:game>
    $<TARGET_OBJECTS:network>
    $<TARGET_OBJECTS:room>
    $<TARGET_OBJECTS:database>
)

# 简化依赖
target_link_libraries(sanguosha_server
    PRIVATE 
        Boost::system
        protobuf::libprotobuf
        SQLite::SQLite3
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
)