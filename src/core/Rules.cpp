#include "core/Rules.hpp"
#include <cstdlib>

namespace { // Functions declared here are only accessible within this file

constexpr int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}}; // left, right, up, down

bool isOnBoard(int row, int col) {
    return row >= 0 && row < 9 && col >= 0 && col < 9;
}

bool isThrone(int row, int col) {
    return row == 4 && col == 4;
}

bool isEdge(int row, int col) {
    return row == 0 || row == 8 || col == 0 || col == 8;
}

int campOf(int row, int col) {
    if ((row == 0 || row == 8) && col >= 3 && col <= 5)
        return row == 0 ? 0 : 1;
    if ((col == 0 || col == 8) && row >= 3 && row <= 5)
        return col == 0 ? 2 : 3;
    if (row == 1 && col == 4)
        return 0;
    if (row == 7 && col == 4)
        return 1;
    if (row == 4 && col == 1)
        return 2;
    if (row == 4 && col == 7)
        return 3;
    return -1;
}

} // namespace

bool isCamp(int row, int col) {
    return campOf(row, col) != -1;
}

namespace {

bool isCampCenter(int row, int col) {
    return (row == 0 && col == 4) || (row == 8 && col == 4) || (row == 4 && col == 0) || (row == 4 && col == 8);
}

bool isLegal(int fromRow, int fromCol, int toRow, int toCol) {
    if (isThrone(toRow, toCol))
        return false;
    int targetCamp = campOf(toRow, toCol);
    return targetCamp == -1 || targetCamp == campOf(fromRow, fromCol); // A pawn can move to a camp only if it is already in that same camp
}

bool isWhitePiece(Piece piece) {
    return piece == Piece::White || piece == Piece::King;
}

void addMoves(std::vector<Move> &moves, const BoardMatrix &board, int row, int col) { // Generate all legal moves for a given piece
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

// Checks whether (row, col) can be used to capture a black pawn
bool isHostileToBlack(const BoardMatrix &board, int row, int col) {
    return isWhitePiece(board[row][col]) || isThrone(row, col) || (isCamp(row, col) && !isCampCenter(row, col));
}

// Checks whether (row, col) can be used to capture a white pawn
bool isHostileToWhite(const BoardMatrix &board, int row, int col) {
    return board[row][col] == Piece::Black || isThrone(row, col) || isCamp(row, col);
}

// Checks whether the king (kingRow, kingCol) is captured, checking the other side of the square (beyondRow, beyondCol) for an active capture
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

    return position;
}

std::vector<Move> Position::calculateLegalMoves() const { // Generate legal moves for every piece belonging to the current player
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

int Position::apply(const Move &move) {
    // Apply the move to the board
    const Piece pieceToMove = board[move.fromRow][move.fromCol];
    board[move.toRow][move.toCol] = pieceToMove;
    board[move.fromRow][move.fromCol] = Piece::Empty;

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
            captured++;
        } else if (sideToMove == Side::Black && target == Piece::White && beyondOnBoard && isHostileToWhite(board, beyondRow, beyondCol)) {
            board[row][col] = Piece::Empty;
            captured++;
        } else if (sideToMove == Side::Black && target == Piece::King && isKingCaptured(board, row, col, beyondRow, beyondCol)) {
            outcome = Outcome::BlackWins;
        }
    }

    if (pieceToMove == Piece::King && isEdge(move.toRow, move.toCol))
        outcome = Outcome::WhiteWins;

    sideToMove = sideToMove == Side::White ? Side::Black : Side::White;
    return captured;
}

bool GameHistory::push(const Position &after, bool captured) {
    if (captured) // If a capture occurred, the previous positions are no longer possible to repeat and therefore useless to keep track of
        sinceLastCapture.clear();

    int seen = 0;
    if (after.outcome == Outcome::Ongoing)
        for (const Position &position : sinceLastCapture)
            if (position == after)
                seen++;

    sinceLastCapture.push_back(after);
    return seen > allowedRepetitions;
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

std::string convertToCoord(int row, int col) {
    return char('a' + col) + std::to_string(row + 1);
}
