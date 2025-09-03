add_executable(network_test
    network_test.cpp
)

target_link_libraries(network_test
    server
    ${Boost_LIBRARIES}
    ${GTEST_BOTH_LIBRARIES}
)

add_test(NAME NetworkTest COMMAND network_test)