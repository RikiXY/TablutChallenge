#include "core/Translator.hpp"
#include <iostream>
#include <rapidjson/document.h>
#include <string_view>

std::array<std::array<int, 9>, 9> Translator::translateFromJson(std::string json){
    // std::cout<<json<<std::endl;
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

    rapidjson::Document doc;
    doc.Parse(json.c_str());
    if (doc.HasParseError())
        throw std::runtime_error("Invalid JSON");

    auto& jsonBoard = doc["board"];
    std::string turn = doc["turn"].GetString();

    // ? Dovrei ottimizzarlo confrontando stati precedenti e successivi
    std::array<std::array<int, 9>, 9> board;
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            std::string_view role = jsonBoard[i][j].GetString();  // avrei potuto allocare anche una normale stringa ma dato che GetString() restituisce un puntatore usiamo quello e via
            if(role == "EMPTY") board[i][j] = 0;
            else if (role == "WHITE") board[i][j] = 1;
            else if (role == "BLACK") board[i][j] = 2;
            else if (role == "KING") board[i][j] = 3;
            else throw std::invalid_argument("Anomaly in JSON board detected");
        }
    }
    return board;
}