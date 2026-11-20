#pragma once

#include "Player.hpp"
#include "engine/Search.hpp"
#include <future>

struct NoLegalMoves {};

class Agent : public Player {
private:
    int timeout;
    int threads;
    std::future<SearchResult> pending;

public:
    /**
     * @param timeoutSeconds The time per move given by the server, in seconds
     * @param searchThreads How many threads search the move
     */
    Agent(int timeoutSeconds, int searchThreads);

    /**
     * @brief Searches the best move available, using the timeout minus a safety margin. If the search doesn't return in time, the best
     * move of its last finished depth; an illegal move is replaced by the first legal one. Throws NoLegalMoves if there's no move available
     *
     * @param position The current position
     * @param history The game history (for the repetitions)
     * @param received When the state was received, used by the move timer
     * @return The best move found
     */
    Move chooseMove(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point received) override;
};