#include "agent/Agent.hpp"
#include <cstdlib>
#include <iostream>

Agent::Agent(ServerConnector &gameConnection, std::string role)
    : Player(gameConnection, role) {} // richiama il costruttore della classe madre

std::string convertToCoord(uint8_t row, uint8_t col) {
    std::string riga = std::to_string(row + 1);
    std::string colonna;
    switch (col) {
    case 0:
        colonna = "a";
        break;
    case 1:
        colonna = "b";
        break;
    case 2:
        colonna = "c";
        break;
    case 3:
        colonna = "d";
        break;
    case 4:
        colonna = "e";
        break;
    case 5:
        colonna = "f";
        break;
    case 6:
        colonna = "g";
        break;
    case 7:
        colonna = "h";
        break;
    case 8:
        colonna = "i";
        break;
    default:
        break;
    }
    return colonna + riga;
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

// TODO improve this generation pls
std::vector<std::array<std::string, 2>> generateMoves(BoardMatrix initialState, uint8_t row, uint8_t col) {

    std::vector<std::array<std::string, 2>> allStatesForOnePawn;
    std::array<std::string, 2> newState;

    int directions[4][2] = {
        {0, -1}, // sinistra
        {0, 1},  // destra
        {-1, 0}, // alto
        {1, 0}   // basso
    };

    // ! idk, ho idea che quel break skippi delle combinazioni possibili
    for (auto i : directions) {     // per ogni vettore in directions
        uint8_t toRow = row + i[0]; // il primo valore è i = [0, -1], quindi toRow = row + (-1). uint8_t 0-1=255
        uint8_t toCol = col + i[1];
        while (toRow >= 0 && toRow < 9 && toCol >= 0 && toCol < 9 && initialState[toRow][toCol] == 0) {
            if (!isLegal(row, col, toRow, toCol))
                break;

            newState[0] = convertToCoord(row, col);
            newState[1] = convertToCoord(toRow, toCol);
            allStatesForOnePawn.push_back(newState);

            toRow += i[0];
            toCol += i[1];
        }
    }
    return allStatesForOnePawn;
}

std::vector<std::array<std::string, 2>> Agent::generateTree() { // vettore di "tuple"
    // TreeGenerator generator;
    // generator.generateTree();

    // BoardMatrix = std::array<std::array<uint8_t, 9>, 9>;
    /*
        000222000
        000020000
        000010000
        200010002
        221131122
        200010002
        000010000
        000020000
        000222000
    */
    BoardMatrix initialState = board.getBoardMatrix();

    std::vector<std::array<std::string, 2>> newLayer;

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (role == "WHITE") {
                if (initialState[i][j] == 1 || initialState[i][j] == 3) { // or white pawns or king
                    std::vector<std::array<std::string, 2>> move = generateMoves(initialState, i, j);
                    newLayer.insert(newLayer.end(), move.begin(), move.end());
                }
            } else {
                if (initialState[i][j] == 2) {
                    std::vector<std::array<std::string, 2>> move = generateMoves(initialState, i, j);
                    newLayer.insert(newLayer.end(), move.begin(), move.end());
                }
            }
        }
    }
    std::cout << "Possible moves number: " << newLayer.size() << std::endl;
    return newLayer;
}

Action Agent::chooseMove() {
    std::vector<std::array<std::string, 2>> mosse = generateTree();

    // ! seleziona una mossa casuale disponibile
    int mossa = rand() % mosse.size();
    std::cout << "Chosen move: \n\tfrom " << mosse[mossa][0] << " to " << mosse[mossa][1] << std::endl;

    return Action(mosse[mossa][0], mosse[mossa][1], role);
}