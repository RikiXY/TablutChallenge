#include "agent/Player.hpp"

#include <iostream>

Player::Player(std::string role)
    : role(role) {}

void Player::getGameState(GameState gameState){
    std::cout<<"Recieved board: "<<std::endl;
    board = gameState.getBoard();
    board.showState();
    
    std::cout<<std::endl<<"Recieved turn: ";
    Turn turn = gameState.getTurn();

    myTurn = (turn.getString() == role) ? true : false;
    
    std::cout<<std::endl;
}

bool Player::isMyTurn(){
    return myTurn;
}

void Player::move(ServerConnector& gameConnection, Action action){
    gameConnection.sendAction(action.getString());
    std::cout<<"Action sent"<<std::endl;
}