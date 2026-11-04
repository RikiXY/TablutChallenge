#include "core/Game.hpp"
#include "agent/Agent.hpp"
#include "agent/Human.hpp"
#include "core/Translator.hpp"
#include <iostream>

namespace { // Functions declared here are only accessible within this file

int countPieces(const BoardMatrix &board) {
    int count = 0;
    for (const auto &row : board)
        for (Piece piece : row)
            if (piece != Piece::Empty)
                count++;
    return count;
}

const char *turnName(const Position &position) {
    switch (position.outcome) {
    case Outcome::WhiteWins:
        return "WHITEWIN";
    case Outcome::BlackWins:
        return "BLACKWIN";
    case Outcome::Draw:
        return "DRAW";
    case Outcome::Ongoing:
        break;
    }
    return position.sideToMove == Side::White ? "WHITE" : "BLACK";
}

} // namespace

void Game::record(const Position &newPosition) {
    if (started) {
        const bool captured = countPieces(newPosition.board) < countPieces(position.board);
        history.push(newPosition, captured);
    }
    position = newPosition;
    started = true;
}

const Position &Game::current() const {
    return position;
}

const GameHistory &Game::pastPositions() const {
    return history;
}

void Game::start(ServerConnector &connection, const Config &config) {

    connection.send(nameToJson(config.name));
    std::cout << "Name sent to server" << std::endl;
    std::cout << "The server will respond once the other player has connected" << std::endl;
    std::cout << std::endl
              << "==============================" << std::endl
              << std::endl;

    const Side playerSide = config.side;
    Human humanPlayer;
    Agent agentPlayer(config.timeout);
    Player &player = (config.mode == Mode::Auto) ? static_cast<Player &>(agentPlayer) : static_cast<Player &>(humanPlayer);

    while (true) {
        std::cout << "Waiting for server response..." << std::endl;
        record(parsePosition(connection.receive()));
        std::cout << "State received" << std::endl;
        printBoard(position.board);
        std::cout << std::endl
                  << "Turn: " << turnName(position) << std::endl;

        if (position.outcome != Outcome::Ongoing) {
            const Outcome playerWin = (playerSide == Side::White) ? Outcome::WhiteWins : Outcome::BlackWins;
            if (position.outcome == playerWin)
                std::cout << std::endl
                          << "YOU ARE THE WINNER" << std::endl;
            else if (position.outcome == Outcome::Draw)
                std::cout << std::endl
                          << "Mh. Better luck next time I guess" << std::endl;
            else
                std::cout << std::endl
                          << "SHAME ON YOU" << std::endl;
            break;
        }

        if (position.sideToMove == playerSide) {
            std::cout << "My turn" << std::endl;
            // TODO Implement timer for human players as well
            const Move move = player.chooseMove(position, history);
            connection.send(moveToJson(move, playerSide));
            std::cout << "Move sent" << std::endl;
        } else {
            std::cout << "Opponent's turn" << std::endl;
            std::cout << std::endl
                      << "==============================" << std::endl
                      << std::endl;
        }
    }
}