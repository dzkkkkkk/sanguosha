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

# Hiredis - 手动设置路径
set(Hiredis_INCLUDE_DIRS "/usr/include/hiredis")
set(Hiredis_LIBRARIES "/usr/lib/x86_64-linux-gnu/libhiredis.so")

# SQLite3
find_package(SQLite3 REQUIRED)

# 包含目录
include_directories(
    include 
    ${Boost_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
    ${Hiredis_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/proto/generated
)

# 添加protobuf生成的文件
file(GLOB PROTO_SRCS "${PROJECT_SOURCE_DIR}/proto/generated/*.cc")

# 创建静态库
add_library(sanguosha_server STATIC
    src/network/server.cpp
    src/network/session.cpp
    src/network/message_codec.cpp
    ${PROTO_SRCS}
)

# 创建主可执行文件
add_executable(server src/main.cpp)
target_link_libraries(server PRIVATE sanguosha_server
    ${Boost_LIBRARIES}
    ${Protobuf_LIBRARIES}
    ${Hiredis_LIBRARIES}
    ${SQLite3_LIBRARIES}
    pthread
)

# 启用测试
enable_testing()

# 添加测试子目录
add_subdirectory(test)