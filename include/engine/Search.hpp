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
 * @param threads How many threads search together
 * @param exact true to search every move at full depth and completely
 * @return The best move of the last depth (unfinished if the deadline is reached), the score, the last finished depth, and the nodes searched by all the threads
 */
SearchResult search(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point deadline, int maxDepth = 64, int threads = 1, bool exact = false);

/**
 * @brief Makes the running search return as soon as possible
 */
void stopSearch();

/**
 * @brief The best move of the last depth the current (or last) search finished
 *
 * @return The move, or Move{} if no depth has finished yet
 */
Move completedMove();

/**
 * @brief Counts the CPUs this process is allowed to run on
 *
 * @return The number of CPUs
 */
int usableCpus();

/**
 * @brief Reallocates and clears the transposition table (64 MiB by default)
 *
 * @param megabytes The size of the table in MiB
 */
void resizeTranspositionTable(size_t megabytes);
