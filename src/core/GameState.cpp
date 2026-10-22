#include "core/GameState.hpp"

void GameState::update(std::string rawJson){
    // * Update Board
    currentBoard.updateState(rawJson);

    // * Check turn
    currentTurn.updateState(rawJson);

    if (currentTurn.getTurn() == "WHITEWIN" || currentTurn.getTurn() == "BLACKWIN" || currentTurn.getTurn() == "DRAW")
        finished = true;
}

Board GameState::getBoard(){
    return currentBoard;
}

Turn GameState::getTurn(){
    return currentTurn;
}

bool GameState::isOver(){
    return finished;
}

std::string GameState::winner(){
    if(currentTurn.getTurn() == "WHITEWIN")
        return "WHITE";
    else if(currentTurn.getTurn() == "BLACKWIN")
        return "BLACK";
    else if(currentTurn.getTurn() == "DRAW")
        return "DRAW";
}