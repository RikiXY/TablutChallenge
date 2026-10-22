#include "core/Game.hpp"
#include "agent/Human.hpp"
#include "agent/Agent.hpp"
#include <iostream>

void Game::start(ServerConnector& gameConnection, std::string name, std::string role, std::string modality){

    gameConnection.joinGame(name);
    std::cout<<"Name sent to server"<<std::endl;
    std::cout<<"The server will respond once the other player has connected"<<std::endl;

    GameState currentGameState;
    Board currentBoard;
    Turn currentTurn;

    Agent agent;
    Human player;

    std::cout<<std::endl<<"=============================="<<std::endl<<std::endl;

    do {
        std::cout<<"Waiting for server response..."<<std::endl;
        currentGameState.update(gameConnection.receiveJson());
        std::cout<<"State received"<<std::endl;
        
        currentBoard = currentGameState.getBoard();
        currentTurn = currentGameState.getTurn();
        
        player.getGameState(currentGameState);  // il player riceve sempre subito lo stato aggiornato così può pensare intanto che l'altro giocatore sceglie
        
        // * Gestione turni
        if(currentTurn.getTurn() == role){
            std::cout<<"My turn"<<std::endl;
            
            if(modality == "auto"){
                //TODO selettore automatico 
            }
            else { // manual selection
                std::cin.get();
                player.move(gameConnection, "e3", "f3");
            }
            // ! invio mossa
        }
        else {
            // * aspetto il mio turno
            std::cout<<"Opponent's turn"<<std::endl;
        }
    } while(1);
    

    
     


}