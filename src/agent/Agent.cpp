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

// TODO improve this generation pls
std::vector<std::array<std::string, 2>> Agent::generateMoves(BoardMatrix initialState, uint8_t X, uint8_t Y){
    
    std::vector<std::array<std::string, 2>> allStatesForOnePawn;
    std::array<std::string, 2> newState;

    // check sx
    int i = Y;
    while(i > 0 && initialState[X][i-1] == 0){
        newState[0] = convertToCoord(X, Y);
        newState[1] = convertToCoord(X, i-1);
        allStatesForOnePawn.push_back(newState);
        i--;
    }

    // check sx
    i = Y;
    while(i < 8 && initialState[X][i+1] == 0){
        newState[0] = convertToCoord(X, Y);
        newState[1] = convertToCoord(X, i+1);
        allStatesForOnePawn.push_back(newState);
        i++;
    }

    // check up
    i = X;
    while(i > 0 && initialState[i-1][Y] == 0){
        newState[0] = convertToCoord(X, Y);
        newState[1] = convertToCoord(i-1, Y);
        allStatesForOnePawn.push_back(newState);
        i--;
    }

    // check down
    i = X;
    while(i < 8 && initialState[i+1][Y] == 0){
        newState[0] = convertToCoord(X, Y);
        newState[1] = convertToCoord(i+1, Y);
        allStatesForOnePawn.push_back(newState);
        i++;
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