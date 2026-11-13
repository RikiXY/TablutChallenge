// build/selfplay -depth 13 -seed 1 -games logs/selfplay-1.txt

#include "core/GameHistory.hpp"
#include "core/Position.hpp"
#include "core/Translator.hpp"
#include "engine/Network.hpp"
#include "engine/Search.hpp"
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr int maxPlies = 400;

struct Options {
    int depth = 13;
    int msPerMoveLimit = 10000;
    int games = 1000000;
    int openingPlies = 6;
    uint64_t seed = 1;
    std::string gamesFile;
};

struct GameResult {
    double whiteScore = 0.5; // 1 if White wins, 0 if Black wins, 0.5 if draw
    bool reachedMaxPlies = false;
    std::vector<std::string> moves;
    std::vector<std::string> searchScores; // The score of the search that chose each move, from White's point of view ("-" for the opening)
};

std::vector<Move> randomOpening(int plies, std::mt19937_64 &random) {
    for (int attempt = 0; attempt < 1000; attempt++) { // If a random opening accidentally ends the game, try again
        Position position = Position::start();
        GameHistory history;
        std::vector<Move> moves;
        bool repeated = false;

        // Applies the "plies" random legal moves from the start position
        for (int ply = 0; ply < plies; ply++) {
            std::vector<Move> legalMoves;
            position.calculateLegalMoves(legalMoves);
            if (legalMoves.empty() || position.outcome != Outcome::Ongoing)
                break;
            moves.push_back(legalMoves[random() % legalMoves.size()]);
            const bool captured = position.apply(moves.back()) > 0;
            if (history.push(position, captured)) {
                repeated = true; // A repetition would end the game as a draw
                break;
            }
        }

        // Only accept the opening if the game is still ongoing
        if (!repeated && static_cast<int>(moves.size()) == plies && position.outcome == Outcome::Ongoing)
            return moves;
    }

    throw std::runtime_error("No random opening of " + std::to_string(plies) + " plies found in 1000 attempts");
}

// One engine plays both sides, so the table it keeps from move to move holds the analysis of both
GameResult playGame(const std::vector<Move> &opening, const Options &options) {
    Position position = Position::start();
    GameHistory history;
    GameResult result;
    resizeTranspositionTable(64);

    for (int ply = 0; ply < maxPlies; ply++) {
        // If the side to move has no legal moves, it loses
        if (!position.hasLegalMove()) {
            result.whiteScore = position.sideToMove == Side::White ? 0 : 1;
            return result;
        }

        // Select the next move
        Move move{};
        if (ply < static_cast<int>(opening.size())) {
            move = opening[ply];
            result.searchScores.push_back("-");
        } else {
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(options.msPerMoveLimit);
            const SearchResult searched = search(position, history, deadline, options.depth);
            move = searched.bestMove;
            result.searchScores.push_back(std::to_string(position.sideToMove == Side::White ? searched.score : -searched.score));
        }

        // Check if the move ends the game or causes a repetition
        const bool captured = position.apply(move) > 0;
        result.moves.push_back(moveToText(move));
        if (history.push(position, captured))
            return result;
        if (position.outcome == Outcome::WhiteWins || position.outcome == Outcome::BlackWins) {
            result.whiteScore = position.outcome == Outcome::WhiteWins ? 1 : 0;
            return result;
        }
    }

    result.reachedMaxPlies = true;
    return result;
}

Options parseOptions(int argc, char *argv[]) {
    Options options;
    for (int i = 1; i < argc; i += 2) {
        const std::string_view flag(argv[i]);
        if (i + 1 >= argc)
            throw std::invalid_argument("Missing value for " + std::string(flag));
        const std::string value(argv[i + 1]);
        if (flag == "-depth")
            options.depth = std::stoi(value);
        else if (flag == "-ms")
            options.msPerMoveLimit = std::stoi(value);
        else if (flag == "-count")
            options.games = std::stoi(value);
        else if (flag == "-plies")
            options.openingPlies = std::stoi(value);
        else if (flag == "-seed")
            options.seed = std::stoull(value);
        else if (flag == "-games")
            options.gamesFile = value;
        else
            throw std::invalid_argument("Unknown option: " + std::string(flag));
    }
    if (options.gamesFile.empty())
        throw std::invalid_argument("Usage: selfplay -games FILE [-depth 13] [-ms 10000] [-count 1000000] [-plies 6] [-seed 1]");
    return options;
}

} // namespace

int main(int argc, char *argv[]) {
    Options options;
    try {
        options = parseOptions(argc, argv);
        loadNetworkFromEnvironment();
    } catch (const std::exception &error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    std::ofstream games(options.gamesFile, std::ios::app);
    if (!games) {
        std::cerr << "Cannot open " << options.gamesFile << std::endl;
        return 1;
    }

    std::mt19937_64 random(options.seed);
    try {
        for (int game = 0; game < options.games; game++) {
            const GameResult result = playGame(randomOpening(options.openingPlies, random), options);

            // WHITE_SCORE REACHED_MAX_PLIES MOVES | SCORES
            games << result.whiteScore << ' ' << result.reachedMaxPlies;
            for (const std::string &move : result.moves)
                games << ' ' << move;
            games << " |";
            for (const std::string &score : result.searchScores)
                games << ' ' << score;
            games << '\n';
            games.flush(); // So stopping the process by hand loses only the game being played
        }
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}
