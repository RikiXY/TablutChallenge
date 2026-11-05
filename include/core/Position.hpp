#pragma once

#include "core/Board.hpp"
#include <cstdint>
#include <vector>

enum class Side : uint8_t { White,
                            Black };
enum class Outcome : uint8_t { Ongoing,
                               WhiteWins,
                               BlackWins,
                               Draw };

struct Position {
    BoardMatrix board{};
    Side sideToMove = Side::White;
    Outcome outcome = Outcome::Ongoing;
    uint64_t hash = 0;

    static Position start(); // Static method: callable without an instance of the struct

    std::vector<Move> calculateLegalMoves() const; // Generate legal moves without modifying the position
    int apply(const Move &move);
    void computeHash();
    bool operator==(const Position &) const = default; // Automatically compare all members for equality
};

uint64_t perft(const Position &position, int depth);