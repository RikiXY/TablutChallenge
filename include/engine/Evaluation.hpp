#pragma once

#include "core/Position.hpp"
#include <array>

constexpr int WIN = 30000;
constexpr int WIN_BOUND = WIN - 1000;

// From white's point of view
enum Feature {
    WhitePawns,              // white pawns, king excluded
    BlackPawns,              // black pawns
    KingOpenRays,            // directions (0-4) with a clear line from the king to the edge
    EscapeWhiteToMove,       // 1 if White is to move and has an open line, so it escapes next move
    DoubleEscapeBlackToMove, // 1 if Black is to move facing two or more open lines, it can block only one and thus the king escapes next move
    BlackNextToKing,         // black pawns on the king's 4 neighbouring squares
    CampOrThroneNextToKing,  // camp cells and the throne on the king's 4 neighbouring squares
    KingDistanceFromThrone,  // Manhattan distance between the king and the throne
    BlackCornerGuards,       // black pawns on b3 c2 g2 h3 b7 c8 g8 h7, the squares that guard the corners
    KingMobility,            // squares the king can move to
    KingTwoStep,             // king moves after which the king would have an open line
    WhiteNextToKing,         // white pawns on the king's 4 neighbouring squares
    Constant,                // always 1: the structural advantage of one colour, which matters when comparing a score with a draw (0)
    FeatureCount
};

using Features = std::array<int, FeatureCount>;
using Weights = std::array<int, FeatureCount>;

extern const Weights defaultWeights;

Features computeFeatures(const Position &position);

// From the side to move's point of view
int evaluate(const Position &position, const Weights &weights = defaultWeights);
