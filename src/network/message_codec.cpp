#include "network/message_codec.h"
#include <google/protobuf/io/coded_stream.h>
#include <stdexcept>
#include <arpa/inet.h>  // 用于htonl/ntohl

using namespace Sanguosha::Network;

std::vector<char> MessageCodec::encode(const sanguosha::GameMessage& msg) {
    // 计算消息体大小
    size_t body_size = msg.ByteSizeLong();
    size_t total_size = HEADER_LENGTH + body_size;
    
    std::vector<char> buffer(total_size);
    char* ptr = buffer.data();
    
    // 写入消息头（长度）
    uint32_t net_size = htonl(static_cast<uint32_t>(body_size));
    std::memcpy(ptr, &net_size, HEADER_LENGTH);
    
    // 写入消息体
    msg.SerializeToArray(ptr + HEADER_LENGTH, body_size);
    
    return buffer;
}

sanguosha::GameMessage MessageCodec::decode(const std::vector<char>& buffer) {
    if (buffer.size() < HEADER_LENGTH) {
        throw std::runtime_error("Message too short");
    }
    
    // 解析消息头
    uint32_t net_size;
    std::memcpy(&net_size, buffer.data(), HEADER_LENGTH);
    uint32_t body_size = ntohl(net_size);
    
    // 检查消息长度
    if (buffer.size() < HEADER_LENGTH + body_size) {
        throw std::runtime_error("Incomplete message");
    }
    
    // 解析消息体
    sanguosha::GameMessage msg;
    if (!msg.ParseFromArray(buffer.data() + HEADER_LENGTH, body_size)) {
        throw std::runtime_error("Parse message failed");
    }
    
    return msg;
}