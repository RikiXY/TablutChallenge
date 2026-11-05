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

struct GameHistory {
    int allowedRepetitions = 0;
    std::vector<uint64_t> sinceLastCapture;

    bool push(const Position &after, bool captured);
};

uint64_t perft(const Position &position, int depth);

constexpr int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}}; // left, right, up, down

constexpr bool isOnBoard(int row, int col) {
    return row >= 0 && row < 9 && col >= 0 && col < 9;
}

constexpr bool isThrone(int row, int col) {
    return row == 4 && col == 4;
}

bool isCamp(int row, int col);
