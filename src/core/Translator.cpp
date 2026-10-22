#include "core/Translator.hpp"
#include <iostream>
#include <rapidjson/document.h>
#include <string_view>

/* Fixed JSON:
{
    "board":[
        ["EMPTY","EMPTY","EMPTY","BLACK","BLACK","BLACK","EMPTY","EMPTY","EMPTY"],
        ["EMPTY","EMPTY","EMPTY","EMPTY","BLACK","EMPTY","EMPTY","EMPTY","EMPTY"],
        ["EMPTY","EMPTY","EMPTY","EMPTY","WHITE","EMPTY","EMPTY","EMPTY","EMPTY"],
        ["BLACK","EMPTY","EMPTY","EMPTY","WHITE","EMPTY","EMPTY","EMPTY","BLACK"],
        ["BLACK","BLACK","WHITE","WHITE","KING" ,"WHITE","WHITE","BLACK","BLACK"],
        ["BLACK","EMPTY","EMPTY","EMPTY","WHITE","EMPTY","EMPTY","EMPTY","BLACK"],
        ["EMPTY","EMPTY","EMPTY","EMPTY","WHITE","EMPTY","EMPTY","EMPTY","EMPTY"],
        ["EMPTY","EMPTY","EMPTY","EMPTY","BLACK","EMPTY","EMPTY","EMPTY","EMPTY"],
        ["EMPTY","EMPTY","EMPTY","BLACK","BLACK","BLACK","EMPTY","EMPTY","EMPTY"]
    ],
    "turn":"WHITE"
}
*/

Translator::Translator(std::string json){

    rapidjson::Document doc;
    doc.Parse(json.c_str());
    if (doc.HasParseError())
        throw std::runtime_error("Invalid JSON");

    // ! non controllo la struttura del JSON. Do per scontato che sia ok
    auto& jsonBoard = doc["board"];

    // ? Dovrei ottimizzarlo confrontando stati precedenti e successivi
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            std::string_view role = jsonBoard[i][j].GetString();  // avrei potuto allocare anche una normale stringa ma dato che GetString() restituisce un puntatore usiamo quello e via
            if(role == "EMPTY") board[i][j] = 0;
            else if (role == "WHITE") board[i][j] = 1;
            else if (role == "BLACK") board[i][j] = 2;
            else if (role == "KING") board[i][j] = 3;
            else if (role == "THRONE") board[i][j] = 4;
            else throw std::invalid_argument("Anomaly in JSON board detected");
        }
    }
    turn = doc["turn"].GetString();
}

std::array<std::array<uint8_t, 9>, 9> Translator::getBoard(){
    return board;
}

std::string Translator::getTurn(){
    return turn;
}