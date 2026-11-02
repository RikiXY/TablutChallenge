#include "agent/Agent.hpp"
#include <cstdlib>
#include <iostream>

Move Agent::chooseMove(const Position &position, const GameHistory &history) {
    std::vector<Move> moves = position.calculateLegalMoves();
    std::cout << "Possible moves number: " << moves.size() << std::endl;

    // ! seleziona una mossa casuale disponibile
    const Move chosenMove = moves[rand() % moves.size()];
    const std::string from = convertToCoord(chosenMove.fromRow, chosenMove.fromCol);
    const std::string to = convertToCoord(chosenMove.toRow, chosenMove.toCol);
    std::cout << "Chosen move: \n\tfrom " << from << " to " << to << std::endl;

    return chosenMove;
}