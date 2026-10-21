#include "core/GameState.hpp"

GameState::GameState(std::string rawJson){

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