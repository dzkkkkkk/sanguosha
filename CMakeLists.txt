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

# 添加protobuf生成
set(PROTO_FILE "${PROJECT_SOURCE_DIR}/proto/sanguosha.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

# 包含目录
include_directories(
    include 
    ${Boost_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/proto/generated  # 添加proto生成目录
    ${PROJECT_SOURCE_DIR}/proto            # 添加proto目录
)

# 添加核心模块
add_subdirectory(src/game)
add_subdirectory(src/network)
add_subdirectory(src/room)

# 简化可执行文件
add_executable(sanguosha_server
    src/main.cpp
    ${PROTO_SRCS}  # 添加生成的protobuf源文件
    $<TARGET_OBJECTS:game>
    $<TARGET_OBJECTS:network>
    $<TARGET_OBJECTS:room>
)

# 简化依赖
target_link_libraries(sanguosha_server
    PRIVATE 
        Boost::system
        protobuf::libprotobuf
        pthread
)