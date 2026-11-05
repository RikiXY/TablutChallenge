#pragma once

#include "Player.hpp"

class Human : public Player {
public:
    /**
     * @brief Asks for a move on the terminal
     *
     * @return The move typed by the user
     */
    Move chooseMove(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point received) override;
};