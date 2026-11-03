#pragma once

#include "core/Rules.hpp"
#include <chrono>
#include <cstdint>

struct SearchResult {
    Move bestMove{};
    int score = 0;
    int depth = 0;
    uint64_t nodes = 0;
};

SearchResult search(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point deadline, int maxDepth = 64);
