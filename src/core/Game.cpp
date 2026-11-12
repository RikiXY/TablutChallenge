#include "core/Game.hpp"
#include "agent/Agent.hpp"
#include "agent/Human.hpp"
#include "core/Translator.hpp"
#include <chrono>
#include <iostream>
#include <string>

namespace { // Functions declared here are only accessible within this file

/**
 * @brief Counts the pieces on the board
 *
 * @param position The position
 * @return The number of pieces on the board
 */
int countPieces(const Position &position) {
    return popcount(position.white | position.black | position.king);
}

/**
 * @brief Translates the position outcome into the server's turn
 *
 * @param position The position
 * @return WHITE, BLACK, WHITEWIN, BLACKWIN or DRAW
 */
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
        const bool captured = countPieces(newPosition) < countPieces(position);
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
    Agent agentPlayer(config.timeout, config.threads);
    Player &player = (config.mode == Mode::Auto) ? static_cast<Player &>(agentPlayer) : static_cast<Player &>(humanPlayer);

    while (true) {
        std::cout << "Waiting for server response..." << std::endl;
        const std::string message = connection.receive();
        const auto receivedAt = std::chrono::steady_clock::now();
        record(parsePosition(message));
        std::cout << "State received" << std::endl;
        printBoard(position);
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
            Move move{};
            try {
                move = player.chooseMove(position, history, receivedAt);
            } catch (const NoLegalMoves &) {
                std::cout << std::endl
                          << "No legal moves: the game is lost" << std::endl;
                break;
            }
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