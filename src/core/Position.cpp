#include "core/Position.hpp"
#include <array>
#include <cstdlib>

namespace { // Functions declared here are only accessible within this file

struct ZobristKeys {
    std::array<std::array<uint64_t, 81>, 4> piece{};
    uint64_t whiteToMove = 0;
};

/**
 * @brief Generates the Zobrist keys at compile time with SplitMix64. Taken from https://prng.di.unimi.it/splitmix.c
 *
 * @return The keys
 */
constexpr ZobristKeys makeKeys() {
    uint64_t state = 0x9E3779B97F4A7C15ull;
    auto next = [&state] {
        uint64_t z = (state += 0x9E3779B97F4A7C15ull);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    };
    ZobristKeys keys;
    for (Piece p : {Piece::White, Piece::Black, Piece::King})
        for (uint64_t &key : keys.piece[static_cast<int>(p)])
            key = next();
    keys.whiteToMove = next();
    return keys;
}

constexpr ZobristKeys zobrist = makeKeys();

/**
 * @brief Gets the Zobrist key of a piece on a cell
 *
 * @param piece The piece
 * @param row The row of the cell
 * @param col The column of the cell
 * @return The key (always 0 for an empty cell)
 */
uint64_t key(Piece piece, int row, int col) {
    return zobrist.piece[static_cast<int>(piece)][row * 9 + col];
}

/**
 * @brief Checks whether a piece can stop on a cell
 *
 * @param fromRow The row the piece starts from
 * @param fromCol The column the piece starts from
 * @param toRow The row of the target cell
 * @param toCol The column of the target cell
 * @return true if the piece can stop there
 */
bool isLegal(int fromRow, int fromCol, int toRow, int toCol) {
    if (isThrone(toRow, toCol))
        return false;
    int targetCamp = campOf(toRow, toCol);
    return targetCamp == -1 || targetCamp == campOf(fromRow, fromCol); // A pawn can move to a camp only if it is already in that same camp
}

/**
 * @brief Checks whether a piece belongs to White
 *
 * @param piece The piece
 * @return true for a white pawn or the king
 */
bool isWhitePiece(Piece piece) {
    return piece == Piece::White || piece == Piece::King;
}

/**
 * @brief Generates all legal moves for a given piece
 *
 * @param moves Where to add the moves
 * @param board The board
 * @param row The row of the piece
 * @param col The column of the piece
 */
void addMoves(std::vector<Move> &moves, const BoardMatrix &board, int row, int col) {
    for (const auto &[dRow, dCol] : directions) {
        int toRow = row + dRow;
        int toCol = col + dCol;

        while (isOnBoard(toRow, toCol) && board[toRow][toCol] == Piece::Empty && isLegal(row, col, toRow, toCol)) {
            moves.push_back({row, col, toRow, toCol}); // Initialize a Move object and append it to the vector
            toRow += dRow;
            toCol += dCol;
        }
    }
}

/**
 * @brief Checks whether a cell can be used to capture a black pawn
 *
 * @param board The board
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true for a white piece, the throne or a camp cell that isn't a center
 */
bool isHostileToBlack(const BoardMatrix &board, int row, int col) {
    return isWhitePiece(board[row][col]) || isThrone(row, col) || (isCamp(row, col) && !isCampCenter(row, col));
}

/**
 * @brief Checks whether a cell can be used to capture a white pawn
 *
 * @param board The board
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true for a black pawn, the throne or a camp cell
 */
bool isHostileToWhite(const BoardMatrix &board, int row, int col) {
    return board[row][col] == Piece::Black || isThrone(row, col) || isCamp(row, col);
}

/**
 * @brief Checks whether the king is captured by the black pawn that just moved next to it
 *
 * @param board The board after the move
 * @param kingRow The row of the king
 * @param kingCol The column of the king
 * @param beyondRow The row of the cell on the other side of the king, seen from the pawn that moved
 * @param beyondCol The column of that cell
 * @return true if the king is captured
 */
bool isKingCaptured(const BoardMatrix &board, int kingRow, int kingCol, int beyondRow, int beyondCol) {
    const bool onOrNextToThrone = std::abs(kingRow - 4) + std::abs(kingCol - 4) <= 1; // Manhattan distance between the king and the throne

    if (onOrNextToThrone) {
        for (const auto &[dRow, dCol] : directions) {
            const int row = kingRow + dRow;
            const int col = kingCol + dCol;
            if (!isThrone(row, col) && board[row][col] != Piece::Black) // If at least one of the sides is neither throne nor a black pawn, the king is not captured
                return false;
        }
        return true;
    }

    return isOnBoard(beyondRow, beyondCol) && (board[beyondRow][beyondCol] == Piece::Black || isCamp(beyondRow, beyondCol));
}

} // namespace

Position Position::start() {
    Position position;

    // Set the black pawns in the camps
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (isCamp(i, j))
                position.board[i][j] = Piece::Black;

    // Set the white pawns in the cross around the king
    for (int i : {2, 3, 5, 6}) {
        position.board[i][4] = Piece::White;
        position.board[4][i] = Piece::White;
    }
    // Set the king in the throne
    position.board[4][4] = Piece::King;

    position.computeHash();
    return position;
}

std::vector<Move> Position::calculateLegalMoves() const {
    std::vector<Move> moves;
    if (outcome != Outcome::Ongoing)
        return moves;

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            const Piece piece = board[row][col];
            const bool belongsToCurrentSide = sideToMove == Side::White ? isWhitePiece(piece) : piece == Piece::Black;
            if (belongsToCurrentSide)
                addMoves(moves, board, row, col);
        }
    }
    return moves;
}

bool Position::hasLegalMove() const {
    if (outcome != Outcome::Ongoing)
        return false;

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            const Piece piece = board[row][col];
            const bool belongsToCurrentSide = sideToMove == Side::White ? isWhitePiece(piece) : piece == Piece::Black;
            if (!belongsToCurrentSide)
                continue;
            for (const auto &[dRow, dCol] : directions) {
                const int toRow = row + dRow;
                const int toCol = col + dCol;
                if (isOnBoard(toRow, toCol) && board[toRow][toCol] == Piece::Empty && isLegal(row, col, toRow, toCol))
                    return true;
            }
        }
    }
    return false;
}

int Position::apply(const Move &move) {
    // Apply the move to the board
    const Piece pieceToMove = board[move.fromRow][move.fromCol];
    board[move.toRow][move.toCol] = pieceToMove;
    board[move.fromRow][move.fromCol] = Piece::Empty;
    hash ^= key(pieceToMove, move.fromRow, move.fromCol) ^ key(pieceToMove, move.toRow, move.toCol);

    // Simulate the server's capture rules locally
    int captured = 0;
    for (const auto &[dRow, dCol] : directions) {
        // (row, col) is the neighbour that may be captured
        const int row = move.toRow + dRow;
        const int col = move.toCol + dCol;
        if (!isOnBoard(row, col))
            continue;
        const Piece target = board[row][col];

        // (beyondRow, beyondCol) is the cell after the neighbour
        const int beyondRow = row + dRow;
        const int beyondCol = col + dCol;
        const bool beyondOnBoard = isOnBoard(beyondRow, beyondCol);

        if (sideToMove == Side::White && target == Piece::Black && beyondOnBoard && isHostileToBlack(board, beyondRow, beyondCol)) {
            board[row][col] = Piece::Empty;
            hash ^= key(target, row, col);
            captured++;
        } else if (sideToMove == Side::Black && target == Piece::White && beyondOnBoard && isHostileToWhite(board, beyondRow, beyondCol)) {
            board[row][col] = Piece::Empty;
            hash ^= key(target, row, col);
            captured++;
        } else if (sideToMove == Side::Black && target == Piece::King && isKingCaptured(board, row, col, beyondRow, beyondCol)) {
            outcome = Outcome::BlackWins;
        }
    }

    if (pieceToMove == Piece::King && isEdge(move.toRow, move.toCol))
        outcome = Outcome::WhiteWins;

    sideToMove = sideToMove == Side::White ? Side::Black : Side::White;
    hash ^= zobrist.whiteToMove;
    return captured;
}

void Position::computeHash() {
    hash = sideToMove == Side::White ? zobrist.whiteToMove : 0;
    for (int row = 0; row < 9; row++)
        for (int col = 0; col < 9; col++)
            hash ^= key(board[row][col], row, col);
}

uint64_t perft(const Position &position, int depth) {
    const std::vector<Move> moves = position.calculateLegalMoves();
    if (depth == 1)
        return moves.size();

    uint64_t count = 0;
    for (const Move &move : moves) {
        Position next = position;
        next.apply(move);
        if (next.outcome == Outcome::Ongoing)
            count += perft(next, depth - 1);
    }
    return count;
}
