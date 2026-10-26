#include "agent/Player.hpp"

#include <iostream>

Player::Player(ServerConnector& gameConnection, std::string role)
    : gameConnection(gameConnection), role(role) {}

void Player::setGameState(GameState gameState){
    std::cout<<"Receieved board: "<<std::endl;
    board = gameState.getBoard();
    board.showState();
    
    std::cout<<std::endl<<"Receieved turn: ";
    Turn turn = gameState.getTurn();
    turn.showState();

    myTurn = (turn.getString() == role) ? true : false;
}

bool Player::isMyTurn(){
    return myTurn;
}

void Player::move(Action action){
    gameConnection.sendAction(action.getString());
    std::cout<<"Action sent"<<std::endl;
}