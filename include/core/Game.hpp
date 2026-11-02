#pragma once

#include "connections/ServerConnector.hpp"
#include "core/Rules.hpp"
#include <string>

class Game {
private:
    Position position;
    GameHistory history;
    bool started = false;

public:
    void record(const Position &newPosition);
    const Position &current() const;
    const GameHistory &pastPositions() const;

    void start(ServerConnector &gameConnection, std::string name, std::string role, std::string mode);
};