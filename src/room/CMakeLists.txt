# Room module CMakeLists.txt
add_library(room OBJECT
    room.cpp
    room_manager.cpp
)

target_include_directories(room
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(room
    PRIVATE
        Boost::system
        protobuf::libprotobuf
)