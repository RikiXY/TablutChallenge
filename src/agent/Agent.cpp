#include "agent/Agent.hpp"
#include "core/Translator.hpp"
#include "engine/Search.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>

Agent::Agent(int timeoutSeconds) : timeoutSeconds(timeoutSeconds) {}

Move Agent::chooseMove(const Position &position, const GameHistory &history) {
    if (position.calculateLegalMoves().empty())
        throw std::runtime_error("No legal moves: the game is lost");

    constexpr int safetyMarginSeconds = 3;
    const auto thinkingTime = std::chrono::seconds(std::max(timeoutSeconds - safetyMarginSeconds, 1));
    const SearchResult result = search(position, history, std::chrono::steady_clock::now() + thinkingTime);

    const Move &move = result.bestMove;
    std::cout << "Chosen move: \n\tfrom " << convertToCoord(move.fromRow, move.fromCol)
              << " to " << convertToCoord(move.toRow, move.toCol) << std::endl;
    std::cout << "\tdepth " << result.depth << ", score " << result.score << ", " << result.nodes << " nodes" << std::endl;
    return move;
}