#include "core/Translator.hpp"
#include <algorithm>
#include <cctype>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
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

    // The structure is checked before reading it
    if (!doc.IsObject() || !doc.HasMember("board") || !doc["board"].IsArray() || doc["board"].Size() != 9 || !doc.HasMember("turn") || !doc["turn"].IsString())
        throw std::invalid_argument("Anomaly in JSON structure detected");
    const auto &jsonBoard = doc["board"];
    for (const auto &row : jsonBoard.GetArray())
        if (!row.IsArray() || row.Size() != 9 || !std::all_of(row.Begin(), row.End(), [](const auto &cell) { return cell.IsString(); }))
            throw std::invalid_argument("Anomaly in JSON board detected");
    Position position;

    for (rapidjson::SizeType i = 0; i < 9; i++) {
        for (rapidjson::SizeType j = 0; j < 9; j++) {
            const std::string_view piece = jsonBoard[i][j].GetString();
            const int square = static_cast<int>(i * 9 + j);
            if (piece == "EMPTY" || piece == "THRONE") // THRONE is the server's name for an empty throne
                position.put(square, Piece::Empty);
            else if (piece == "WHITE")
                position.put(square, Piece::White);
            else if (piece == "BLACK")
                position.put(square, Piece::Black);
            else if (piece == "KING")
                position.put(square, Piece::King);
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

    if (popcount(position.king) > 1 || (position.outcome == Outcome::Ongoing && popcount(position.king) != 1))
        throw std::invalid_argument("Anomaly in JSON board detected: no king or more than one");

    position.computeHash();
    return position;
}

std::string moveToJson(const Move &move, Side side) {
    const std::string from = convertToCoord(move.from);
    const std::string to = convertToCoord(move.to);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    writer.Key("from");
    writer.String(from.c_str());
    writer.Key("to");
    writer.String(to.c_str());
    writer.Key("turn");
    writer.String(side == Side::White ? "WHITE" : "BLACK");
    writer.EndObject();
    return buffer.GetString();
}

std::string nameToJson(const std::string &name) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.String(name.c_str(), static_cast<rapidjson::SizeType>(name.size()));
    return buffer.GetString();
}

std::string convertToCoord(int square) {
    return char('a' + square % 9) + std::to_string(square / 9 + 1);
}

bool parseSquare(const std::string &text, int &square) {
    if (text.size() != 2)
        return false;
    const int col = std::tolower(static_cast<unsigned char>(text[0])) - 'a';
    const int row = text[1] - '1';
    square = row * 9 + col;
    return row >= 0 && row < 9 && col >= 0 && col < 9;
}

std::string moveToText(const Move &move) {
    return convertToCoord(move.from) + convertToCoord(move.to);
}

bool parseMove(const std::string &text, Move &move) {
    int from, to;
    if (text.size() != 4 || !parseSquare(text.substr(0, 2), from) || !parseSquare(text.substr(2), to))
        return false;
    move = makeMove(from, to);
    return true;
}