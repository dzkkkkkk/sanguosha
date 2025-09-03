cmake_minimum_required(VERSION 3.12)
project(SanguoshaServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
)

# 添加可执行文件
add_executable(server
    src/main.cpp
    src/network/server.cpp
)

# 链接库
target_link_libraries(server
    ${Boost_LIBRARIES}
    ${Protobuf_LIBRARIES}
    ${Hiredis_LIBRARIES}
    ${SQLite3_LIBRARIES}
    pthread
)