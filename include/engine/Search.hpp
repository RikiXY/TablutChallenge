#pragma once

#include "core/GameHistory.hpp"
#include "core/Position.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>

struct SearchResult {
    Move bestMove{};
    int score = 0;
    int depth = 0;
    uint64_t nodes = 0;
};

/**
 * @brief Searches the best move with iterative deepening until the deadline or maxDepth
 *
 * @param position The position to search
 * @param history The game history
 * @param deadline The deadline (when to stop). Depth 1 always finishes
 * @param maxDepth The maximum depth
 * @return The best move of the last finished depth, with its score, the depth and the nodes searched
 */
SearchResult search(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point deadline, int maxDepth = 64);

/**
 * @brief Reallocates and clears the transposition table (64 MiB by default)
 *
 * @param megabytes The size of the table in MiB
 */
void resizeTranspositionTable(size_t megabytes);
