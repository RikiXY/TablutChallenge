#include "agent/Agent.hpp"
#include "core/Translator.hpp"
#include "engine/Search.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>

Agent::Agent(int timeout) : timeoutSeconds(timeout) {}

Move Agent::chooseMove(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point startingTime) {
    if (position.calculateLegalMoves().empty())
        throw NoLegalMoves{};

    // The search budget is the timeout minus a reserve of 3% of it for safety, kept between 350 ms and 2 s
    const int timeoutMs = timeoutSeconds * 1000;
    const int reserveMs = std::clamp(timeoutSeconds * 30, 350, 2000);
    const auto budget = std::chrono::milliseconds(std::max(timeoutMs - reserveMs, 50));
    const SearchResult result = search(position, history, startingTime + budget);

    const Move &move = result.bestMove;
    std::cout << "Chosen move: \n\tfrom " << convertToCoord(move.fromRow, move.fromCol)
              << " to " << convertToCoord(move.toRow, move.toCol) << std::endl;
    std::cout << "\tdepth " << result.depth << ", score " << result.score << ", " << result.nodes << " nodes" << std::endl;
    return move;
}