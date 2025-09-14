# Network module CMakeLists.txt
add_library(network OBJECT
    message_codec.cpp
    server.cpp
    session.cpp
)

target_include_directories(network
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(network
    PRIVATE
        Boost::system
        protobuf::libprotobuf
)