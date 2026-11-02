#pragma once

#include "GameState.hpp"
#include "connections/ServerConnector.hpp"

class Game {
public:
    void start(ServerConnector &, std::string, std::string, std::string);
};