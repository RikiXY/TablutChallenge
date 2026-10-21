#include "core/GameState.hpp"

void GameState::update(std::string rawJson){
    // * Update Board
    currentBoard.updateState(rawJson);

    // * Check turn
    currentTurn.updateState(rawJson);
}

Board GameState::getBoard(){
    return currentBoard;
}

Turn GameState::getTurn(){
    return currentTurn;
}