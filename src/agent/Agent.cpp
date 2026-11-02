#include "agent/Agent.hpp"
#include "core/Rules.hpp"
#include <cstdlib>
#include <iostream>

Agent::Agent(ServerConnector &gameConnection, std::string role)
    : Player(gameConnection, role) {} // richiama il costruttore della classe madre

Action Agent::chooseMove() {
    const Position position{board.getBoardMatrix(), role == "WHITE" ? Side::White : Side::Black};
    std::vector<Move> moves = position.calculateLegalMoves();
    std::cout << "Possible moves number: " << moves.size() << std::endl;

    // ! seleziona una mossa casuale disponibile
    const Move &chosenMove = moves[rand() % moves.size()];
    const std::string from = convertToCoord(chosenMove.fromRow, chosenMove.fromCol);
    const std::string to = convertToCoord(chosenMove.toRow, chosenMove.toCol);
    std::cout << "Chosen move: \n\tfrom " << from << " to " << to << std::endl;

    return Action(from, to, role);
}