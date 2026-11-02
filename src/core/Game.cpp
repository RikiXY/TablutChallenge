#include "core/Game.hpp"
#include "agent/Agent.hpp"
#include "agent/Human.hpp"
#include "core/Translator.hpp"
#include <iostream>

void Game::start(ServerConnector &gameConnection, std::string name, std::string role, std::string modality) {

    gameConnection.joinGame(name);
    std::cout << "Name sent to server" << std::endl;
    std::cout << "The server will respond once the other player has connected" << std::endl;
    std::cout << std::endl
              << "==============================" << std::endl
              << std::endl;

    Translator translator;
    GameState currentGameState;

    Human humanPlayer(gameConnection, role);
    Agent agentPlayer(gameConnection, role);

    while (1) {
        std::cout << "Waiting for server response..." << std::endl;
        translator.update(gameConnection.receiveJson());
        currentGameState.update(translator.getBoard(), translator.getTurn());
        std::cout << "State received" << std::endl;

        if (modality == "auto") {
            agentPlayer.setGameState(currentGameState);
            if (agentPlayer.isMyTurn()) {
                std::cout << "My turn" << std::endl;
                // ! Far partire il timer qui (non usiamo 60s, teniamo un margine)
                // ! la logica di selezione non è ancora implementata
                agentPlayer.move(agentPlayer.chooseMove());
            } else {
                std::cout << "Opponent's turn" << std::endl;
                std::cout << std::endl
                          << "==============================" << std::endl
                          << std::endl;
            }
        } else {
            humanPlayer.setGameState(currentGameState); // il player riceve sempre subito lo stato aggiornato così può pensare intanto che l'altro giocatore sceglie
            if (humanPlayer.isMyTurn()) {
                // ! Far partire il timer qui (non usiamo 60s, teniamo un margine)
                std::cout << "My turn" << std::endl;
                humanPlayer.move(humanPlayer.chooseMove());
            } else
                std::cout << "Opponent's turn" << std::endl;
        }

        if (currentGameState.isOver()) {
            if (currentGameState.winner() == role)
                std::cout << std::endl
                          << "YOU ARE THE WINNER" << std::endl;
            else if (currentGameState.winner() == "DRAW")
                std::cout << std::endl
                          << "Mh. Better luck next time I guess" << std::endl;
            else
                std::cout << std::endl
                          << "SHAME ON YOU" << std::endl;
            break;
        }
    }
}