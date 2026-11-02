#include "agent/Agent.hpp"
#include <cstdlib>
#include <iostream>

Agent::Agent(ServerConnector &gameConnection, std::string role)
    : Player(gameConnection, role) {} // richiama il costruttore della classe madre

// (row, col) to server notation: column a-i + row 1-9, e.g. (0, 4) -> "e1"
std::string convertToCoord(int row, int col) {
    return char('a' + col) + std::to_string(row + 1);
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

bool isLegal(int fromRow, int fromCol, int toRow, int toCol) {
    if (toRow == 4 && toCol == 4) // Throne, not a legal target
        return false;
    int targetCamp = campOf(toRow, toCol);
    return targetCamp == -1 || targetCamp == campOf(fromRow, fromCol); // The pawn can move to a camp only if it is already in that same camp
}

bool isOnBoard(int row, int col) {
    return row >= 0 && row < 9 && col >= 0 && col < 9;
}

std::vector<Move> generateMoves(const BoardMatrix &board, int row, int col) {
    constexpr int directions[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}}; // left, right, up, down
    std::vector<Move> moves;

    for (const auto &[dRow, dCol] : directions) {
        int toRow = row + dRow;
        int toCol = col + dCol;

        while (isOnBoard(toRow, toCol) && board[toRow][toCol] == Piece::Empty && isLegal(row, col, toRow, toCol)) {
            moves.push_back({row, col, toRow, toCol});
            toRow += dRow;
            toCol += dCol;
        }
    }
    return moves;
}

std::vector<Move> Agent::generateAllMoves() {
    const BoardMatrix currentBoard = board.getBoardMatrix();
    const bool isWhite = role == "WHITE";
    std::vector<Move> moves;

    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            const Piece piece = currentBoard[row][col];
            const bool isMine = isWhite ? (piece == Piece::White || piece == Piece::King) : piece == Piece::Black;
            if (!isMine)
                continue;

            const auto pieceMoves = generateMoves(currentBoard, row, col);
            moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
        }
    }
    std::cout << "Possible moves number: " << moves.size() << std::endl;
    return moves;
}

Action Agent::chooseMove() {
    std::vector<Move> mosse = generateAllMoves();

    // ! seleziona una mossa casuale disponibile
    const Move &mossa = mosse[rand() % mosse.size()];
    const std::string from = convertToCoord(mossa.fromRow, mossa.fromCol);
    const std::string to = convertToCoord(mossa.toRow, mossa.toCol);
    std::cout << "Chosen move: \n\tfrom " << from << " to " << to << std::endl;

    return Action(from, to, role);
}