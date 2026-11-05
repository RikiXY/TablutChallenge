#pragma once

#include "config/Config.hpp"
#include "connections/ServerConnector.hpp"
#include "core/GameHistory.hpp"
#include "core/Position.hpp"

class Game {
private:
    Position position;
    GameHistory history;
    bool started = false;

public:
    /**
     * @brief Saves a new position, adding it to the history if it's not the first one
     *
     * @param newPosition The position received from the server
     */
    void record(const Position &newPosition);

    /**
     * @return The current position
     */
    const Position &current() const;

    /**
     * @return The game history
     */
    const GameHistory &pastPositions() const;

    /**
     * @brief Plays the whole game: sends the name, then reads the reply from the server and answers with a move
     *
     * @param connection The connection to the server
     * @param config The command line options
     */
    void start(ServerConnector &connection, const Config &config);
};