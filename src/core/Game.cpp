#include "core/Game.hpp"
#include <iostream>

void Game::start(ServerConnector& gameConnection, std::string name, std::string role, std::string modality){

    gameConnection.joinGame(name);
    std::cout<<"Name sent to server"<<std::endl;
    std::cout<<"The server will respond once the other player has connected"<<std::endl;

    GameState currentGameState;
    Board currentBoard;
    Turn currentTurn;
    do {
        std::cout<<"Waiting for server response..."<<std::endl;
        currentGameState.update(gameConnection.receiveJson());
        std::cout<<"State received"<<std::endl;
        
        std::cout<<std::endl<<"=============================="<<std::endl<<std::endl;
        
        currentBoard = currentGameState.getBoard();
        std::cout<<"Current board:"<<std::endl;
        currentBoard.showState();

        currentTurn = currentGameState.getTurn();
        std::cout<<std::endl<<"Current turn: ";
        currentTurn.showState();
        
        // * Gestione turni
        if(currentTurn.getTurn() == role){
            std::cout<<"My turn"<<std::endl;
            
            if(modality == "auto"){
                //TODO selettore automatico 
            }
            else {
                
            }
            // ! invio mossa
        }
        else {
            // * aspetto il mio turno
            std::cout<<"Opponent's turn"<<std::endl;
        }
    } while(1);
    

    
     


}