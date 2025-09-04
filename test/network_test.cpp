#include <gtest/gtest.h>
#include "room/room.h" // 包含必要头文件
#include "room/room_manager.h"

using namespace Sanguosha::Room;

class RoomTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 使用完整类型替代RoomPtr
        room = std::make_shared<Room>(1001);
    }
    
    std::shared_ptr<Room> room; // 使用完整类型
};

TEST_F(RoomTest, AddPlayer) {
    EXPECT_TRUE(room->addPlayer(1));
    EXPECT_TRUE(room->addPlayer(2));
    EXPECT_FALSE(room->addPlayer(1)); // 重复添加
    EXPECT_EQ(room->playerCount(), 2);
}

TEST_F(RoomTest, RemovePlayer) {
    room->addPlayer(1);
    room->addPlayer(2);
    EXPECT_TRUE(room->removePlayer(1));
    EXPECT_FALSE(room->removePlayer(3)); // 移除不存在的玩家
    EXPECT_EQ(room->playerCount(), 1);
}

TEST_F(RoomTest, StateTransition) {
    // 初始状态为等待
    EXPECT_EQ(room->state(), Room::State::WAITING);
    
    // 添加玩家后仍为等待状态
    room->addPlayer(1);
    room->addPlayer(2);
    EXPECT_EQ(room->state(), Room::State::WAITING);
    
    // 开始选将
    EXPECT_TRUE(room->startChoosing());
    EXPECT_EQ(room->state(), Room::State::CHOOSING);
    
    // 开始游戏
    EXPECT_TRUE(room->startGame());
    EXPECT_EQ(room->state(), Room::State::PLAYING);
    
    // 结束游戏
    EXPECT_TRUE(room->endGame());
    EXPECT_EQ(room->state(), Room::State::ENDED);
    
    // 尝试从结束状态回退
    EXPECT_FALSE(room->startChoosing());
}

class RoomManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mgr = &RoomManager::Instance();
    }
    
    RoomManager* mgr;
};

TEST_F(RoomManagerTest, CreateRoom) {
    uint32_t roomId = mgr->createRoom();
    EXPECT_NE(roomId, 0);
    
    auto room = mgr->getRoom(roomId);
    EXPECT_NE(room, nullptr);
}

TEST_F(RoomManagerTest, JoinLeaveRoom) {
    uint32_t roomId = mgr->createRoom();
    
    EXPECT_TRUE(mgr->joinRoom(roomId, 1));
    EXPECT_TRUE(mgr->joinRoom(roomId, 2));
    
    auto room = mgr->getRoom(roomId);
    EXPECT_EQ(room->playerCount(), 2);
    
    EXPECT_TRUE(mgr->leaveRoom(roomId, 1));
    EXPECT_EQ(room->playerCount(), 1);
}

TEST_F(RoomManagerTest, MatchPlayers) {
    std::vector<uint32_t> players1 = {1, 2, 3};
    std::vector<uint32_t> players2 = {4, 5};
    
    uint32_t roomId1 = mgr->matchPlayers(players1);
    uint32_t roomId2 = mgr->matchPlayers(players2);
    
    // 应该分配到同一个房间
    EXPECT_EQ(roomId1, roomId2);
    
    auto room = mgr->getRoom(roomId1);
    EXPECT_EQ(room->playerCount(), 5);
}