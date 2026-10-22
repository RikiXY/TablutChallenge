#pragma once

#include "connections/ServerConnector.hpp"
#include "GameState.hpp"

class Game{
    public:
    void start(ServerConnector&, std::string, std::string, std::string);
};