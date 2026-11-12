#pragma once

#include "core/Position.hpp"
#include "engine/Network.hpp"
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
    KingEdgeDistance,        // distance from the king to the nearest edge
    BlackNearKing,           // black pawns within 2 squares of the king in both directions
    WhiteNearKing,           // white pawns within 2 squares of the king in both directions
    BlackOnKingLines,        // black pawns on the king's row or column
    KingOnThrone,            // 1 if the king is on the throne
    // Piece-square tables
    PstKing,
    PstWhite = PstKing + 15,
    PstBlack = PstWhite + 15,
    FeatureCount = PstBlack + 15
};

using Features = std::array<int, FeatureCount>;
using Weights = std::array<int, FeatureCount>;

extern const Weights defaultWeights;

/**
 * @brief Computes the features of a position from White's point of view
 *
 * @param position The position
 * @return The value of each feature
 */
Features computeFeatures(const Position &position);

/**
 * @brief Counts the directions in which the king has a clear line to the edge
 *
 * @param position The position
 * @return The number of open lines, 0 if there is no king
 */
int kingOpenRays(const Position &position);

/**
 * @brief Calculates the linear part of the score without the network
 *
 * @param position The position
 * @param weights The weight of each feature
 * @return The score from White's point of view
 */
int linearScore(const Position &position, const Weights &weights = defaultWeights);

/**
 * @brief Scores a position from the side to move's point of view
 *
 * @param position The position
 * @param weights The weight of each feature
 * @param accumulator The network's hidden layer of the position, nullptr to calculate it from scratch. Ignored if no network is loaded
 * @return The score, positive if it's good for the side to move
 */
int evaluate(const Position &position, const Weights &weights = defaultWeights, const Accumulator *accumulator = nullptr);