#pragma once

#include "Board.hpp"
#include "Translator.hpp"
#include "GameState.hpp"
#include "connections/ServerConnector.hpp"

class Game{
    public:
    void start(ServerConnection& gameConnection, std::string name, std::string role);
};