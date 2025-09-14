# 添加游戏模块
add_library(game
    game_instance.cpp
    player.cpp
    # 添加其他必要文件
)

# 包含目录
target_include_directories(game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${Protobuf_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(game PRIVATE
    ${Protobuf_LIBRARIES}
    room  # 添加room模块依赖
)