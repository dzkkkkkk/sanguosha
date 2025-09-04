cmake_minimum_required(VERSION 3.12)

# 使用系统安装的 GTest（无需下载）
find_package(GTest REQUIRED)

# 添加网络测试可执行文件
add_executable(network_test
    network_test.cpp
)

# 链接库 - 使用系统安装的GTest
target_link_libraries(network_test PRIVATE
    sanguosha_server  # 这是主项目创建的静态库
    GTest::gtest_main  # 使用系统安装的GTest
    ${Boost_LIBRARIES}
    ${Protobuf_LIBRARIES}
    pthread
)

# 包含目录
target_include_directories(network_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/proto/generated
    # 不再需要GTest源码目录
)

# 添加测试
include(GoogleTest)
gtest_discover_tests(network_test)