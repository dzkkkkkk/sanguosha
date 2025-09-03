cmake_minimum_required(VERSION 3.12)
project(SanguoshaServer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖库
find_package(Boost 1.71 REQUIRED COMPONENTS system asio)
find_package(Protobuf REQUIRED)
find_package(Hiredis REQUIRED)
find_package(SQLite3 REQUIRED)

# 包含头文件目录
include_directories(include ${Boost_INCLUDE_DIRS} 
                    ${Protobuf_INCLUDE_DIRS}
                    ${Hiredis_INCLUDE_DIRS}
                    ${SQLite3_INCLUDE_DIRS})

# 添加可执行文件
add_executable(server
    src/main.cpp
    src/network/server.cpp
    # 后续添加其他源文件
)

# 链接库
target_link_libraries(server
    ${Boost_LIBRARIES}
    ${Protobuf_LIBRARIES}
    ${Hiredis_LIBRARIES}
    ${SQLite3_LIBRARIES}
    pthread
)

# 添加测试目标
enable_testing()
add_subdirectory(test)