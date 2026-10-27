#include "agent/Agent.hpp"
#include <iostream>
#include <cstdlib>

Agent::Agent(ServerConnector& gameConnection, std::string role) 
    : Player(gameConnection, role) {}  // richiama il costruttore della classe madre

std::string convertToCoord(uint8_t X, uint8_t Y){
    std::string riga = std::to_string(X+1);
    std::string colonna;
    switch (Y) {
    case 0:
        colonna = "a"; break;
    case 1:
        colonna = "b"; break;
    case 2:
        colonna = "c"; break;
    case 3:
        colonna = "d"; break;
    case 4:
        colonna = "e"; break;
    case 5:
        colonna = "f"; break;
    case 6:
        colonna = "g"; break;
    case 7:
        colonna = "h"; break;
    case 8:
        colonna = "i"; break;        
    default:
        break;
    }
    return colonna + riga;
}

bool isIllegal(uint8_t currentX, uint8_t currentY, uint8_t newX, uint8_t newY){
    if(newX == 4 && newY == 4)
        return true;

    if((currentX == 0 && currentY == 4) ||
        (currentX == 4 && currentY == 0) ||
        (currentX == 4 && currentY == 8) ||
        (currentX == 8 && currentY == 4)){
            return false;
        }
    switch(newX){
        case 0:
            return (newY == 3 || newY == 5) ? true : false; break;
        case 1:
            return (newY == 4) ? true : false; break;
        case 3:
            return (newY == 0 || newY == 8) ? true : false; break;
        case 4:
            return (newY == 1 || newY == 7) ? true : false; break;
        case 5:
            return (newY == 0 || newY == 8) ? true : false; break;
        case 7:
            return (newY == 4) ? true : false; break;
        case 8:
            return (newY == 3 || newY == 5) ? true : false; break;
        default:
            return false; break;
    }
}

// TODO improve this generation pls
std::vector<std::array<std::string, 2>> generateMoves(BoardMatrix initialState, uint8_t X, uint8_t Y){
    
    std::vector<std::array<std::string, 2>> allStatesForOnePawn;
    std::array<std::string, 2> newState;

    int directions[4][2] = {
        {0, -1},  // sinistra
        {0, 1},   // destra
        {-1, 0},  // alto
        {1, 0}    // basso
    };

    // ! idk, ho idea che quel break skippi delle combinazioni possibili
    for (auto i : directions){  // per ogni vettore in directions 
        uint8_t newX = X + i[0];  // il primo valore è i = [0, -1], quindi newX = X + (-1). uint8_t 0-1=255
        uint8_t newY = Y + i[1];
        while(newX >= 0 && newX < 9 && newY >= 0 && newY < 9 && initialState[newX][newY] == 0){
            if (isIllegal(X, Y, newX, newY))
                break;

            newState[0] = convertToCoord(X, Y);
            newState[1] = convertToCoord(newX, newY);
            allStatesForOnePawn.push_back(newState);

            newX += i[0];
            newY += i[1];
        }
    }
    return allStatesForOnePawn;
}

std::vector<std::array<std::string, 2>> Agent::generateTree(){ // vettore di "tuple"
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
    
    for(int i = 0; i < 9; i++){
        for (int j = 0; j < 9; j++){
            if (role == "WHITE"){
                if (initialState[i][j] == 1 || initialState[i][j] == 3){  // or white pawns or king
                    std::vector<std::array<std::string, 2>> move = generateMoves(initialState, i, j);
                    newLayer.insert(newLayer.end(), move.begin(), move.end());
                }
            }
            else{
                if (initialState[i][j] == 2){ 
                    std::vector<std::array<std::string, 2>> move = generateMoves(initialState, i, j);
                    newLayer.insert(newLayer.end(), move.begin(), move.end());
                }
            }
        }
    } 
    std::cout<<"Possible moves number: "<<newLayer.size()<<std::endl;
    return newLayer;
}

Action Agent::chooseMove(){
    std::vector<std::array<std::string, 2>> mosse = generateTree();

    // ! seleziona una mossa casuale disponibile
    int mossa = rand() % mosse.size();
    std::cout<<"Chosen move: \n\tfrom "<<mosse[mossa][0]<<" to "<<mosse[mossa][1]<<std::endl;
    
    return Action(mosse[mossa][0], mosse[mossa][1], role);
}