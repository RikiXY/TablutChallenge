#pragma once

#include "core/Board.hpp"
#include <cstdint>
#include <string>
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

std::string convertToCoord(int row, int col);

bool isCamp(int row, int col);
