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

    Human humanPlayer(role);
    Agent agentPlayer(role);

    std::cout<<std::endl<<"=============================="<<std::endl<<std::endl;

    while(1) {
        std::cout<<"Waiting for server response..."<<std::endl;
        currentGameState.update(gameConnection.receiveJson());
        std::cout<<"State received"<<std::endl;
        
        currentBoard = currentGameState.getBoard();
        currentTurn = currentGameState.getTurn();

        if(modality == "auto"){
            agentPlayer.getGameState(currentGameState);
            if(agentPlayer.isMyTurn())
                // ! la logica di selezione non è ancora implementata 
                agentPlayer.move(gameConnection, agentPlayer.chooseMove());  
        }
        else{
            humanPlayer.getGameState(currentGameState);  // il player riceve sempre subito lo stato aggiornato così può pensare intanto che l'altro giocatore sceglie
            if(humanPlayer.isMyTurn())
                humanPlayer.move(gameConnection, humanPlayer.chooseMove());
            else 
                std::cout<<"Opponent's turn"<<std::endl;
        }
        
        if(currentGameState.isOver()){
            if(currentGameState.winner() == role)
                std::cout<<std::endl<<"YOU ARE THE WINNER"<<std::endl;
            else if(currentGameState.winner() == "DRAW")
                std::cout<<std::endl<<"Mh. Better luck next time I guess"<<std::endl;
            else
                std::cout<<std::endl<<"SHAME ON YOU"<<std::endl;
            break;
        }
        
    }
}