#pragma once
#include <string>

namespace sanguosha {

class Player {
public:
    Player(uint32_t id, const std::string& username);

    uint32_t getId() const;
    const std::string& getUsername() const;

private:
    uint32_t id_;
    std::string username_;
};

} // namespace sanguosha