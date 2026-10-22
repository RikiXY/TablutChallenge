#include "agent/Human.hpp"

#include <iostream>
#include "core/Action.hpp"

void Human::move(ServerConnector& gameConnection, std::string from, std::string to){
    // ! manca la validazione perché diamo per scontato, almeno per questi test, che il player sappia che sta facendo
    Action action(from, to, role);
    std::cout<<action.getString()<<std::endl;
    gameConnection.sendAction(action.getString());
    std::cout<<"sent"<<std::endl;
}

void Human::getGameState(GameState gameState){
    
    std::cout<<"Recieved board: "<<std::endl;
    gameState.getBoard().showState();
    boardMatrix = gameState.getBoard().getBoardMatrix();
    
    std::cout<<std::endl<<"Recieved turn: ";
    gameState.getTurn().showState();
    role = gameState.getTurn().getTurn();
    std::cout<<std::endl;
}