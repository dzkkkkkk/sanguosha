# Game module CMakeLists.txt
add_library(game OBJECT
    game_instance.cpp
    player.cpp
)

target_include_directories(game
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(game
    PRIVATE
        protobuf::libprotobuf
)