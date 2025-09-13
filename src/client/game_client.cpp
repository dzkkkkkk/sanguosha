#include "client/game_client.h"
#include "network/connection.h"
#include <thread>
#include <iostream>

namespace sanguosha {

class GameClient::Impl {
public:
    Impl() : connected(false), loggedIn(false), running(false) {}
    
    std::shared_ptr<Connection> connection;
    uint32_t playerId;
    uint32_t roomId;
    bool connected;
    bool loggedIn;
    bool running;
    
    std::function<void(const GameState&)> gameStateCallback;
    std::function<void(const GameStart&)> gameStartCallback;
    
    void handleMessage(const GameMessage& message) {
        switch (message.type()) {
            case GAME_STATE:
                if (gameStateCallback) {
                    gameStateCallback(message.game_state());
                }
                break;
            case GAME_START:
                if (gameStartCallback) {
                    gameStartCallback(message.game_start());
                }
                break;
            case LOGIN_RESPONSE:
                handleLoginResponse(message.login_response());
                break;
            case ROOM_RESPONSE:
                handleRoomResponse(message.room_response());
                break;
            default:
                std::cout << "Received unhandled message type: " << message.type() << std::endl;
        }
    }
    
    void handleLoginResponse(const LoginResponse& response) {
        loggedIn = response.success();
        if (loggedIn) {
            playerId = response.user_id();
            std::cout << "Login successful. Player ID: " << playerId << std::endl;
        } else {
            std::cout << "Login failed: " << response.error_message() << std::endl;
        }
    }
    
    void handleRoomResponse(const RoomResponse& response) {
        if (response.success()) {
            roomId = response.room_info().room_id();
            std::cout << "Room operation successful. Room ID: " << roomId << std::endl;
        } else {
            std::cout << "Room operation failed: " << response.error_message() << std::endl;
        }
    }
};

GameClient::GameClient() : pimpl_(std::make_unique<Impl>()) {}

GameClient::~GameClient() = default;

bool GameClient::connect(const std::string& host, uint16_t port) {
    pimpl_->connection = std::make_shared<Connection>();
    pimpl_->connected = pimpl_->connection->connect(host, port);
    return pimpl_->connected;
}

bool GameClient::login(const std::string& username, const std::string& password) {
    if (!pimpl_->connected) return false;
    
    LoginRequest request;
    request.set_username(username);
    request.set_password(password);
    
    GameMessage message;
    message.set_type(LOGIN_REQUEST);
    message.mutable_login_request()->CopyFrom(request);
    
    pimpl_->connection->send(message);
    return true;
}

bool GameClient::createRoom() {
    RoomRequest request;
    request.set_action(CREATE_ROOM);
    
    GameMessage message;
    message.set_type(ROOM_REQUEST);
    message.mutable_room_request()->CopyFrom(request);
    
    pimpl_->connection->send(message);
    return true;
}

bool GameClient::joinRoom(uint32_t roomId) {
    RoomRequest request;
    request.set_action(JOIN_ROOM);
    request.set_room_id(roomId);
    
    GameMessage message;
    message.set_type(ROOM_REQUEST);
    message.mutable_room_request()->CopyFrom(request);
    
    pimpl_->connection->send(message);
    return true;
}

void GameClient::setGameStateCallback(std::function<void(const GameState&)> callback) {
    pimpl_->gameStateCallback = callback;
}

void GameClient::setGameStartCallback(std::function<void(const GameStart&)> callback) {
    pimpl_->gameStartCallback = callback;
}

void GameClient::run() {
    pimpl_->running = true;
    while (pimpl_->running) {
        pimpl_->handleIncomingMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void GameClient::stop() {
    pimpl_->running = false;
}

}