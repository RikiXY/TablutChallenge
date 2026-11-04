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

    static Position start();

    std::vector<Move> calculateLegalMoves() const;
    int apply(const Move &move);
    bool operator==(const Position &) const = default;
};

struct GameHistory {
    int allowedRepetitions = 0;
    std::vector<Position> sinceLastCapture;

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
