#include "engine/Evaluation.hpp"
#include "engine/Network.hpp"
#include <cstdlib>

const Weights defaultWeights = {
    191,  // WhitePawns
    -129, // BlackPawns
    253,  // KingOpenRays
    8000, // EscapeWhiteToMove
    4000, // DoubleEscapeBlackToMove
    -74,  // BlackNextToKing
    -33,  // CampOrThroneNextToKing
    65,   // KingDistanceFromThrone
    -47,  // BlackCornerGuards
    -11,  // KingMobility
    30,   // KingTwoStep
    -41,  // WhiteNextToKing
    1     // Constant
};

namespace {

/**
 * @brief Checks whether a cell stops the king
 *
 * @param board The board
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true if the king can't go there or past it
 */
bool blocksKing(const BoardMatrix &board, int row, int col) {
    const Piece piece = board[row][col];
    return (piece != Piece::Empty && piece != Piece::King) || isCamp(row, col) || isThrone(row, col);
}

/**
 * @brief Checks whether a cell guards a corner (b3 b7 h3 h7 c2 c8 g2 g8)
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true if it's one of those cells
 */
bool isCornerGuard(int row, int col) {
    return ((row == 2 || row == 6) && (col == 1 || col == 7)) || ((row == 1 || row == 7) && (col == 2 || col == 6));
}

struct Ray {
    int length = 0;
    bool reachesEdge = false;
};

/**
 * @brief Creates a ray from the king in a direction
 *
 * @param board The board
 * @param kingRow The row of the king
 * @param kingCol The column of the king
 * @param dRow The row step of the direction
 * @param dCol The column step of the direction
 * @return How many cells the king can move along the line and whether it reaches the edge
 */
Ray kingRay(const BoardMatrix &board, int kingRow, int kingCol, int dRow, int dCol) {
    Ray ray;
    int row = kingRow + dRow;
    int col = kingCol + dCol;
    while (isOnBoard(row, col) && !blocksKing(board, row, col)) {
        ray.length++;
        row += dRow;
        col += dCol;
    }
    ray.reachesEdge = ray.length > 0 && !isOnBoard(row, col);
    return ray;
}

/**
 * @brief Counts how many directions have a clear line to the edge from the king's cell
 *
 * @param board The board
 * @param row The row of the king
 * @param col The column of the king
 * @return The number of open lines
 */
int openRays(const BoardMatrix &board, int row, int col) {
    int count = 0;
    for (const auto &[dRow, dCol] : directions)
        if (kingRay(board, row, col, dRow, dCol).reachesEdge)
            count++;
    return count;
}

} // namespace

int kingOpenRays(const Position &position) {
    for (int row = 0; row < 9; row++)
        for (int col = 0; col < 9; col++)
            if (position.board[row][col] == Piece::King)
                return openRays(position.board, row, col);
    return 0;
}

Features computeFeatures(const Position &position) {
    Features features{};
    features[Constant] = 1;
    int kingRow = -1;
    int kingCol = -1;

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            const Piece piece = position.board[row][col];
            if (piece == Piece::White)
                features[WhitePawns]++;
            else if (piece == Piece::Black)
                features[BlackPawns]++;
            else if (piece == Piece::King) {
                kingRow = row;
                kingCol = col;
            }
        }
    }
    if (kingRow == -1) // only in a finished game (captured king): no king features
        return features;

    for (const auto &[dRow, dCol] : directions) {
        const Ray ray = kingRay(position.board, kingRow, kingCol, dRow, dCol);
        features[KingMobility] += ray.length;
        if (ray.reachesEdge)
            features[KingOpenRays]++;
        for (int step = 1; step <= ray.length; step++)
            if (openRays(position.board, kingRow + step * dRow, kingCol + step * dCol) > 0)
                features[KingTwoStep]++;

        const int row = kingRow + dRow;
        const int col = kingCol + dCol;
        if (!isOnBoard(row, col))
            continue;
        if (position.board[row][col] == Piece::Black)
            features[BlackNextToKing]++;
        else if (position.board[row][col] == Piece::White)
            features[WhiteNextToKing]++;
        if (isCamp(row, col) || isThrone(row, col))
            features[CampOrThroneNextToKing]++;
    }

    features[KingDistanceFromThrone] = std::abs(kingRow - 4) + std::abs(kingCol - 4);

    for (int row = 0; row < 9; row++)
        for (int col = 0; col < 9; col++)
            if (isCornerGuard(row, col) && position.board[row][col] == Piece::Black)
                features[BlackCornerGuards]++;

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
