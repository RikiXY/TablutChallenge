#pragma once

#include "config/Config.hpp"
#include "connections/ServerConnector.hpp"
#include "core/Rules.hpp"

class Game {
private:
    Position position;
    GameHistory history;
    bool started = false;

public:
    void record(const Position &newPosition);
    const Position &current() const;
    const GameHistory &pastPositions() const;

    void start(ServerConnector &connection, const Config &config);
};