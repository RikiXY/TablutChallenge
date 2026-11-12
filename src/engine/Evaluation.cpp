#include "engine/Evaluation.hpp"
#include "engine/Network.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>

const Weights defaultWeights = {
    178,  // WhitePawns
    -126, // BlackPawns
    189,  // KingOpenRays
    8000, // EscapeWhiteToMove
    4000, // DoubleEscapeBlackToMove
    -7,   // BlackNextToKing
    44,   // CampOrThroneNextToKing
    146,  // KingDistanceFromThrone
    -54,  // BlackCornerGuards
    -3,   // KingMobility
    14,   // KingTwoStep
    -24,  // WhiteNextToKing
    -94,  // Constant
    51,   // KingEdgeDistance
    -65,  // BlackNearKing
    36,   // WhiteNearKing
    -40,  // BlackOnKingLines
    144,  // KingOnThrone
    // PstKing
    0,
    0,
    -33,
    0,
    -68,
    48,
    0,
    -118,
    18,
    -40,
    0,
    0,
    98,
    59,
    -7,
    // PstWhite
    -16,
    38,
    38,
    82,
    -15,
    -29,
    0,
    14,
    -7,
    -10,
    0,
    0,
    -21,
    -26,
    0,
    // PstBlack
    72,
    34,
    -9,
    19,
    -2,
    -101,
    6,
    132,
    -33,
    11,
    -32,
    23,
    104,
    130,
    0,
};

namespace {

// Per-square tables of the features
struct FeatureTables {
    std::array<int8_t, 81> squareClass{}; // The piece-square class
    std::array<Bitboard, 81> near{};      // The squares within 2 rows and 2 columns, the square excluded
    std::array<Bitboard, 81> lines{};     // The row and the column, the square excluded
    Bitboard cornerGuards = 0;            // b3 b7 h3 h7 c2 c8 g2 g8
};

/**
 * @brief Builds the tables at compile time
 *
 * @return The tables
 */
constexpr FeatureTables makeFeatureTables() {
    FeatureTables featureTables;
    for (int square = 0; square < 81; square++) {
        const int row = square / 9;
        const int col = square % 9;
        const int r = std::min(row, 8 - row);
        const int c = std::min(col, 8 - col);
        const int low = std::min(r, c);
        const int high = std::max(r, c);
        featureTables.squareClass[square] = static_cast<int8_t>(high * (high + 1) / 2 + low);
        if (((row == 2 || row == 6) && (col == 1 || col == 7)) || ((row == 1 || row == 7) && (col == 2 || col == 6)))
            featureTables.cornerGuards |= bit(square);
        for (int other = 0; other < 81; other++) {
            if (other == square)
                continue;
            const int otherRow = other / 9;
            const int otherCol = other % 9;
            if (otherRow >= row - 2 && otherRow <= row + 2 && otherCol >= col - 2 && otherCol <= col + 2)
                featureTables.near[square] |= bit(other);
            if (otherRow == row || otherCol == col)
                featureTables.lines[square] |= bit(other);
        }
    }
    return featureTables;
}

constexpr FeatureTables featureTables = makeFeatureTables();

/**
 * @brief Finds the cells that stop the king
 *
 * @param position The position
 * @return The blockers
 */
Bitboard kingBlockers(const Position &position) {
    return position.white | position.black | tables.camps | bit(THRONE);
}

/**
 * @brief Counts how many directions have a clear line to the edge from a square
 *
 * @param square The square of the king
 * @param blockers The cells that stop the king
 * @return The number of open lines
 */
int openRays(int square, Bitboard blockers) {
    int count = 0;
    for (int direction = 0; direction < 4; direction++)
        if (tables.rayLength[square][direction] > 0 && !(tables.rayMask[square][direction] & blockers))
            count++;
    return count;
}

} // namespace

int kingOpenRays(const Position &position) {
    return position.king ? openRays(position.kingSquare(), kingBlockers(position)) : 0;
}

Features computeFeatures(const Position &position) {
    Features features{};
    features[Constant] = 1;
    features[WhitePawns] = popcount(position.white);
    features[BlackPawns] = popcount(position.black);
    for (Bitboard pawns = position.white; pawns; pawns &= pawns - 1)
        features[PstWhite + featureTables.squareClass[lowestSquare(pawns)]]++;
    for (Bitboard pawns = position.black; pawns; pawns &= pawns - 1)
        features[PstBlack + featureTables.squareClass[lowestSquare(pawns)]]++;
    if (!position.king) // only in a finished game (captured king): no king features
        return features;

    const int king = position.kingSquare();
    const int kingRow = king / 9;
    const int kingCol = king % 9;
    features[PstKing + featureTables.squareClass[king]] = 1;

    const Bitboard blockers = kingBlockers(position);
    for (int direction = 0; direction < 4; direction++) {
        for (int i = 0; i < tables.rayLength[king][direction]; i++) {
            const int square = tables.ray[king][direction][i];
            if (blockers & bit(square))
                break;
            features[KingMobility]++;
            if (openRays(square, blockers) > 0)
                features[KingTwoStep]++;
        }
    }
    features[KingOpenRays] = openRays(king, blockers);

    const Bitboard neighbours = tables.neighbours[king];
    features[BlackNextToKing] = popcount(position.black & neighbours);
    features[WhiteNextToKing] = popcount(position.white & neighbours);
    features[CampOrThroneNextToKing] = popcount((tables.camps | bit(THRONE)) & neighbours);

    features[KingDistanceFromThrone] = std::abs(kingRow - 4) + std::abs(kingCol - 4);
    features[KingEdgeDistance] = std::min(std::min(kingRow, 8 - kingRow), std::min(kingCol, 8 - kingCol));
    features[KingOnThrone] = king == THRONE;

    features[BlackCornerGuards] = popcount(position.black & featureTables.cornerGuards);
    features[BlackNearKing] = popcount(position.black & featureTables.near[king]);
    features[WhiteNearKing] = popcount(position.white & featureTables.near[king]);
    features[BlackOnKingLines] = popcount(position.black & featureTables.lines[king]);

    features[EscapeWhiteToMove] = position.sideToMove == Side::White && features[KingOpenRays] > 0;
    features[DoubleEscapeBlackToMove] = position.sideToMove == Side::Black && features[KingOpenRays] > 1;

    return features;
}

int linearScore(const Position &position, const Weights &weights) {
    const Features features = computeFeatures(position);
    int whiteScore = 0;
    for (int i = 0; i < FeatureCount; i++)
        whiteScore += weights[i] * features[i];
    return whiteScore;
}

int evaluate(const Position &position, const Weights &weights, const Accumulator *accumulator) {
    const int network = accumulator ? networkScore(*accumulator) : networkScore(position);
    const int whiteScore = linearScore(position, weights) + network;
    return position.sideToMove == Side::White ? whiteScore : -whiteScore;
}
