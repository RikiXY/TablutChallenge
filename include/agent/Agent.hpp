#pragma once

#include "Player.hpp"

struct NoLegalMoves {};

class Agent : public Player {
private:
    int timeout;
    int threads;

public:
    /**
     * @param timeoutSeconds The time per move given by the server, in seconds
     * @param searchThreads How many threads search the move
     */
    Agent(int timeoutSeconds, int searchThreads);

    /**
     * @brief Searches the best move available, using the timeout minus a safety margin. Throws NoLegalMoves if there's no move available
     *
     * @param position The current position
     * @param history The game history (for the repetitions)
     * @param received When the state was received, used by the move timer
     * @return The best move found
     */
    Move chooseMove(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point received) override;
};