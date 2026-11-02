#include "core/Translator.hpp"
#include <rapidjson/document.h>
#include <stdexcept>
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

Position parsePosition(const std::string &json) {
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    if (doc.HasParseError())
        throw std::runtime_error("Invalid JSON");

    // ! non controllo la struttura del JSON. Do per scontato che sia ok
    const auto &jsonBoard = doc["board"];
    Position position;

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            const std::string_view piece = jsonBoard[i][j].GetString();
            if (piece == "EMPTY" || piece == "THRONE") // THRONE is the server's name for an empty throne
                position.board[i][j] = Piece::Empty;
            else if (piece == "WHITE")
                position.board[i][j] = Piece::White;
            else if (piece == "BLACK")
                position.board[i][j] = Piece::Black;
            else if (piece == "KING")
                position.board[i][j] = Piece::King;
            else
                throw std::invalid_argument("Anomaly in JSON board detected");
        }
    }

    const std::string_view turn = doc["turn"].GetString();
    if (turn == "WHITE")
        position.sideToMove = Side::White;
    else if (turn == "BLACK")
        position.sideToMove = Side::Black;
    else if (turn == "WHITEWIN")
        position.outcome = Outcome::WhiteWins;
    else if (turn == "BLACKWIN")
        position.outcome = Outcome::BlackWins;
    else if (turn == "DRAW")
        position.outcome = Outcome::Draw;
    else
        throw std::invalid_argument("Anomaly in JSON turn detected: " + std::string(turn));

    return position;
}