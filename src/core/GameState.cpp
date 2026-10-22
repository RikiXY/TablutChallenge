#include "core/GameState.hpp"

void GameState::update(std::string rawJson){
    // * Update Board
    currentBoard.updateState(rawJson);

    // * Check turn
    currentTurn.updateState(rawJson);

    if (currentTurn.getString() == "WHITEWIN" || currentTurn.getString() == "BLACKWIN" || currentTurn.getString() == "DRAW")
        finished = true;
    else 
        finished = false;
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
    if(currentTurn.getString() == "WHITEWIN")
        return "WHITE";
    else if(currentTurn.getString() == "BLACKWIN")
        return "BLACK";
    else if(currentTurn.getString() == "DRAW")
        return "DRAW";
    else return "";
}