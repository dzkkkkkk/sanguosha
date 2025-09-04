#include <gtest/gtest.h>
#include "network/server.h"
#include "network/message_codec.h"
#include "sanguosha.pb.h"

TEST(MessageCodecTest, EncodeDecode) {
    sanguosha::GameMessage msg;
    msg.set_type(sanguosha::LOGIN_REQUEST);
    auto* login = msg.mutable_login_request();
    login->set_username("test");
    login->set_password("123");
    
    // 编码
    auto buffer = Sanguosha::Network::MessageCodec::encode(msg);
    
    // 解码
    auto decoded = Sanguosha::Network::MessageCodec::decode(buffer);
    
    // 验证
    EXPECT_EQ(decoded.type(), sanguosha::LOGIN_REQUEST);
    EXPECT_EQ(decoded.login_request().username(), "test");
    EXPECT_EQ(decoded.login_request().password(), "123");
}

TEST(SessionTest, LoginHandling) {
    // 这里可以添加模拟连接和消息处理的测试
    // 需要模拟boost::asio::ip::tcp::socket
    // 或者使用真实连接进行集成测试
}