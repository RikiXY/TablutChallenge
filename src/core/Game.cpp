#include "core/Game.hpp"
#include <iostream>

void Game::start(ServerConnection& gameConnection, std::string name, std::string role){

    gameConnection.joinGame(name);
    std::cout<<"Name sent to server"<<std::endl;
    std::cout<<"The server will respond once the other player has connected"<<std::endl;
    std::cout<<"Waiting for server response..."<<std::endl;
    
    GameState currentGameState(gameConnection.receiveJson());
    std::cout<<"Initial state recieved"<<std::endl;

    std::cout<<"=============================="<<std::endl;
    
    std::cout<<"Current board:"<<std::endl;
    Board currentBoard = currentGameState.getBoard();
    currentBoard.showState();

    Turn currentTurn = currentGameState.getTurn();
    std::cout<<std::endl<<"Current turn:";
    currentTurn.showState();

    // Gestione turni
    if(currentTurn.getTurn() == role){
        // ! selettore mossa
        // ! invio mossa
    }
    else {
        // * aspetto il mio turno
    }
}