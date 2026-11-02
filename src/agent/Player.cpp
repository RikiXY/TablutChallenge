#include "agent/Player.hpp"

#include <iostream>

Player::Player(ServerConnector &gameConnection, std::string role)
    : gameConnection(gameConnection), role(role) {}

void Player::move(Action action) {
    gameConnection.sendAction(action.getString());
    std::cout << "Action sent" << std::endl;
}