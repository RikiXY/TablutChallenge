#pragma once

#include "connections/ServerConnector.hpp"
#include "core/Action.hpp"
#include "core/Rules.hpp"
#include <string>

class Player {
protected:
    ServerConnector &gameConnection;
    std::string role;

public:
    Player(ServerConnector &gameConnection, std::string role);
    virtual ~Player() = default;
    // metodo virtuale puro che deve essere per forza definito dalle classi figlie
    virtual Action chooseMove(const Position &position, const GameHistory &history) = 0;
    void move(Action action);
};