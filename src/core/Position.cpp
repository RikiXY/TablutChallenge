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

// After a move and before the captures
struct BoardAfterMove {
    const BoardMatrix &board;
    const Move &move;

    /**
     * @brief Reads a cell
     *
     * @param row The row of the cell
     * @param col The column of the cell
     * @return What stands on it after the move
     */
    Piece at(int row, int col) const {
        if (row == move.toRow && col == move.toCol)
            return board[move.fromRow][move.fromCol];
        if (row == move.fromRow && col == move.fromCol)
            return Piece::Empty;
        return board[row][col];
    }
};

/**
 * @brief Checks whether a cell can be used to capture a black pawn
 *
 * @param board The board after the move
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true for a white piece, the throne or a camp cell that isn't a center
 */
bool isHostileToBlack(const BoardAfterMove &board, int row, int col) {
    return isWhitePiece(board.at(row, col)) || isThrone(row, col) || (isCamp(row, col) && !isCampCenter(row, col));
}

/**
 * @brief Checks whether a cell can be used to capture a white pawn
 *
 * @param board The board after the move
 * @param row The row of the cell
 * @param col The column of the cell
 * @return true for a black pawn, the throne or a camp cell
 */
bool isHostileToWhite(const BoardAfterMove &board, int row, int col) {
    return board.at(row, col) == Piece::Black || isThrone(row, col) || isCamp(row, col);
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
bool isKingCaptured(const BoardAfterMove &board, int kingRow, int kingCol, int beyondRow, int beyondCol) {
    const bool onOrNextToThrone = std::abs(kingRow - 4) + std::abs(kingCol - 4) <= 1; // Manhattan distance between the king and the throne

    if (onOrNextToThrone) {
        for (const auto &[dRow, dCol] : directions) {
            const int row = kingRow + dRow;
            const int col = kingCol + dCol;
            if (!isThrone(row, col) && board.at(row, col) != Piece::Black) // If at least one of the sides is neither throne nor a black pawn, the king is not captured
                return false;
        }
        return true;
    }

    return isOnBoard(beyondRow, beyondCol) && (board.at(beyondRow, beyondCol) == Piece::Black || isCamp(beyondRow, beyondCol));
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
    moves.reserve(128);
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

std::vector<Move> Position::calculateTacticalMoves() const {
    std::vector<Move> moves;
    if (outcome != Outcome::Ongoing)
        return moves;
    const auto isOwn = [&](Piece piece) { return sideToMove == Side::White ? isWhitePiece(piece) : piece == Piece::Black; };

    // Finds all the empty cells that are next to an opponent's piece (the only viable ones for a capture)
    std::array<std::array<bool, 9>, 9> nearOpponent{};
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            if (board[row][col] == Piece::Empty || isOwn(board[row][col]))
                continue;
            for (const auto &[dRow, dCol] : directions) {
                const int r = row + dRow;
                const int c = col + dCol;
                if (isOnBoard(r, c) && board[r][c] == Piece::Empty)
                    nearOpponent[r][c] = true;
            }
        }
    }

    std::vector<Move> candidates;
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            if (!nearOpponent[row][col])
                continue;
            for (const auto &[dRow, dCol] : directions) {
                // Find the first piece of the side to move in this direction
                int fromRow = row + dRow;
                int fromCol = col + dCol;
                while (isOnBoard(fromRow, fromCol) && board[fromRow][fromCol] == Piece::Empty) {
                    fromRow += dRow;
                    fromCol += dCol;
                }
                if (!isOnBoard(fromRow, fromCol) || !isOwn(board[fromRow][fromCol]))
                    continue;
                // Check that every cell between the start and the end is legal
                bool reachable = true;
                int r = row;
                int c = col;
                while (reachable && (r != fromRow || c != fromCol)) {
                    reachable = isLegal(fromRow, fromCol, r, c);
                    r += dRow;
                    c += dCol;
                }
                if (reachable)
                    candidates.push_back({fromRow, fromCol, row, col});
            }
        }
    }

    // White wins by moving the king to the edge
    if (sideToMove == Side::White) {
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                if (board[row][col] != Piece::King)
                    continue;
                for (const auto &[dRow, dCol] : directions) {
                    int r = row + dRow;
                    int c = col + dCol;
                    while (isOnBoard(r, c) && board[r][c] == Piece::Empty && isLegal(row, col, r, c)) {
                        if (isEdge(r, c) && !nearOpponent[r][c])
                            candidates.push_back({row, col, r, c});
                        r += dRow;
                        c += dCol;
                    }
                }
            }
        }
    }

    for (const Move &move : candidates) {
        const MoveEffects moveEffects = effects(move);
        if (moveEffects.captured > 0 || moveEffects.outcome != Outcome::Ongoing)
            moves.push_back(move);
    }
    return moves;
}

MoveEffects Position::effects(const Move &move) const {
    // Simulate the server's capture rules locally
    const BoardAfterMove after{board, move};
    MoveEffects result;
    for (int direction = 0; direction < 4; direction++) {
        const auto [dRow, dCol] = directions[direction];
        // (row, col) is the neighbour that may be captured
        const int row = move.toRow + dRow;
        const int col = move.toCol + dCol;
        if (!isOnBoard(row, col))
            continue;
        const Piece target = after.at(row, col);

        // (beyondRow, beyondCol) is the cell after the neighbour
        const int beyondRow = row + dRow;
        const int beyondCol = col + dCol;
        const bool beyondOnBoard = isOnBoard(beyondRow, beyondCol);

        const bool capturesBlack = sideToMove == Side::White && target == Piece::Black && beyondOnBoard && isHostileToBlack(after, beyondRow, beyondCol);
        const bool capturesWhite = sideToMove == Side::Black && target == Piece::White && beyondOnBoard && isHostileToWhite(after, beyondRow, beyondCol);
        if (capturesBlack || capturesWhite) {
            result.capturedDirections = static_cast<uint8_t>(result.capturedDirections | 1 << direction);
            result.captured++;
        } else if (sideToMove == Side::Black && target == Piece::King && isKingCaptured(after, row, col, beyondRow, beyondCol)) {
            result.outcome = Outcome::BlackWins;
        }
    }

    if (after.at(move.toRow, move.toCol) == Piece::King && isEdge(move.toRow, move.toCol))
        result.outcome = Outcome::WhiteWins;
    return result;
}

int Position::apply(const Move &move) {
    return apply(move, effects(move));
}

uint64_t Position::hashAfterQuietMove(const Move &move) const {
    const Piece piece = board[move.fromRow][move.fromCol];
    return hash ^ key(piece, move.fromRow, move.fromCol) ^ key(piece, move.toRow, move.toCol) ^ zobrist.whiteToMove;
}

int Position::apply(const Move &move, const MoveEffects &effects) {
    // Apply the move to the board
    const Piece pieceToMove = board[move.fromRow][move.fromCol];
    board[move.toRow][move.toCol] = pieceToMove;
    board[move.fromRow][move.fromCol] = Piece::Empty;
    hash ^= key(pieceToMove, move.fromRow, move.fromCol) ^ key(pieceToMove, move.toRow, move.toCol);

    // Remove the captured pawns
    for (int direction = 0; direction < 4; direction++) {
        if (!(effects.capturedDirections & (1 << direction)))
            continue;
        const int row = move.toRow + directions[direction][0];
        const int col = move.toCol + directions[direction][1];
        hash ^= key(board[row][col], row, col);
        board[row][col] = Piece::Empty;
    }

    if (effects.outcome != Outcome::Ongoing)
        outcome = effects.outcome;
    sideToMove = sideToMove == Side::White ? Side::Black : Side::White;
    hash ^= zobrist.whiteToMove;
    return effects.captured;
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
