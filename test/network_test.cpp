#include <gtest/gtest.h>
#include "network/server.h"

TEST(ServerTest, BasicInitialization) {
    Sanguosha::Network::Server server;
    ASSERT_NO_THROW(server.start(9527));
}