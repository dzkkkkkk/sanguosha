#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "network/session.h"
#include "room/room_manager.h"

class RoomIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化房间管理器
        auto& roomMgr = Sanguosha::Room::RoomManager::Instance();
        
        // 创建模拟IO上下文
        io_ = std::make_shared<boost::asio::io_context>();
        roomMgr.setIoContext(*io_);
        roomMgr.startCleanupTask();
    }
    
    std::shared_ptr<boost::asio::io_context> io_;
};

TEST_F(RoomIntegrationTest, FullRoomLifecycle) {
    auto& roomMgr = Sanguosha::Room::RoomManager::Instance();
    
    // 创建房间
    uint32_t roomId = roomMgr.createRoom();
    auto room = roomMgr.getRoom(roomId);
    ASSERT_NE(room, nullptr);
    
    // 添加玩家
    roomMgr.joinRoom(roomId, 1);
    roomMgr.joinRoom(roomId, 2);
    EXPECT_EQ(room->playerCount(), 2);
    
    // 开始游戏
    room->startChoosing();
    room->startGame();
    EXPECT_EQ(room->state(), Room::State::PLAYING);
    
    // 结束游戏
    room->endGame();
    EXPECT_EQ(room->state(), Room::State::ENDED);
    
    // 模拟时间流逝（5分钟）
    io_->run_for(std::chrono::minutes(5));
    
    // 房间应该被清理
    room = roomMgr.getRoom(roomId);
    EXPECT_EQ(room, nullptr);
}