#pragma once
#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include "sanguosha.pb.h"

namespace Sanguosha {
namespace Network {

class MessageCodec {
public:
    // 消息头长度（4字节）
    static constexpr size_t HEADER_LENGTH = sizeof(uint32_t);
    
    // 序列化消息
    static std::vector<char> encode(const sanguosha::GameMessage& msg);
    
    // 反序列化消息
    static sanguosha::GameMessage decode(const std::vector<char>& buffer);
};

} // namespace Network
} // namespace Sanguosha