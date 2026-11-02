#include "core/Translator.hpp"
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

void Translator::update(std::string json) {

    rapidjson::Document doc;
    doc.Parse(json.c_str());
    if (doc.HasParseError())
        throw std::runtime_error("Invalid JSON");

    // ! non controllo la struttura del JSON. Do per scontato che sia ok
    auto &jsonBoard = doc["board"];

    BoardMatrix board;

    // ? Dovrei ottimizzarlo confrontando stati precedenti e successivi
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            std::string_view role = jsonBoard[i][j].GetString(); // avrei potuto allocare anche una normale stringa ma dato che GetString() restituisce un puntatore usiamo quello e via
            if (role == "EMPTY")
                board[i][j] = Piece::Empty;
            else if (role == "WHITE")
                board[i][j] = Piece::White;
            else if (role == "BLACK")
                board[i][j] = Piece::Black;
            else if (role == "KING")
                board[i][j] = Piece::King;
            else if (role == "THRONE") // the server's name for an empty throne
                board[i][j] = Piece::Empty;
            else
                throw std::invalid_argument("Anomaly in JSON board detected");
        }
    }
    transaltedBoard.setBoard(board);

    translatedTurn.setTurn(doc["turn"].GetString());
}

Board Translator::getBoard() {
    return transaltedBoard;
}

Turn Translator::getTurn() {
    return translatedTurn;
}