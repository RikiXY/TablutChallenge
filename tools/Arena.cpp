// build/arena -a build/player -b build/player-base -ms 100 -c 4 -games logs/games.txt

#include "core/Rules.hpp"
#include "core/Translator.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

constexpr int maxPlies = 400;
constexpr double llrBound = 2.944; // ln((1 - beta) / alpha) with alpha = beta = 0.05
// alpha = probability of rejecting H0 when it is true (false positive)
// beta = probability of accepting H0 when it is false (false negative)

struct Options {
    std::string engineA;
    std::string engineB;
    int msPerMove = 100;
    int parallelGames = 4;
    int gamePairs = 1000;
    int openingPlies = 6;
    uint64_t seed = 1;
    double elo0 = 0;  // H0: A is this much stronger than B (0 being the same strength)
    double elo1 = 10; // H1: A is this much stronger than B
    std::string gamesFile;
};

struct GameResult {
    double whiteScore = 0.5; // 1 if White wins, 0 if Black wins, 0.5 if draw
    bool failed = false;
    std::vector<std::string> moves;
};

std::vector<Move> randomOpening(int plies, uint64_t seed) {
    std::mt19937_64 random(seed);

    for (int attempt = 0; attempt < 1000; attempt++) { // If a random opening accidentally ends the game, try again
        Position position = Position::start();
        GameHistory history;
        std::vector<Move> moves;
        bool repeated = false;

        // Applies the "plies" random legal moves from the start position
        for (int ply = 0; ply < plies; ply++) {
            const std::vector<Move> legalMoves = position.calculateLegalMoves();
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

Move askEngine(const std::string &engine, int msPerMove, const std::vector<std::string> &moves) {
    // Build the command line to ask the engine giving the list of moves played so far
    std::string command = engine + " think " + std::to_string(msPerMove);
    for (const std::string &move : moves)
        command += " " + move;

    // Start the engine and read its output
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr)
        throw std::runtime_error("Cannot start " + engine);
    std::string output;
    char buffer[256];
    while (std::fgets(buffer, sizeof buffer, pipe) != nullptr)
        output += buffer;
    const int status = pclose(pipe);

    // Extracts the move from the output and checks if it is legal
    std::istringstream words(output);
    std::string keyword;
    std::string moveString;
    words >> keyword >> moveString;
    Move move{};
    if (status != 0 || keyword != "bestmove" || !parseMove(moveString, move))
        throw std::runtime_error(engine + " failed, it answered: " + output);
    return move;
}

GameResult playGame(const std::string &white, const std::string &black, const std::vector<Move> &opening, int msPerMove) {
    Position position = Position::start();
    GameHistory history;
    GameResult result;

    for (int ply = 0; ply < maxPlies; ply++) {
        const std::vector<Move> legal = position.calculateLegalMoves();

        // If the side to move has no legal moves, it loses
        if (legal.empty()) {
            result.whiteScore = position.sideToMove == Side::White ? 0 : 1;
            return result;
        }

        // Select the next move
        const std::string &engine = position.sideToMove == Side::White ? white : black;
        const Move move = ply < static_cast<int>(opening.size()) ? opening[ply] : askEngine(engine, msPerMove, result.moves);
        if (std::find(legal.begin(), legal.end(), move) == legal.end())
            throw std::runtime_error(engine + " played the illegal move " + moveToText(move));

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

    result.failed = true;
    return result;
}

// Results from A's point of view
struct Stats {
    std::array<int, 5> pairCounts{}; // The number of pairs where A scored 0, 0.25, 0.5, 0.75, 1 (average of the two games)
    int wins = 0;
    int draws = 0;
    int losses = 0;
    int failed = 0;

    void addGame(double scoreA, bool gameFailed) {
        if (scoreA == 1)
            wins++;
        else if (scoreA == 0)
            losses++;
        else
            draws++;
        failed += gameFailed;
    }
};

double eloToScore(double elo) {
    return 1 / (1 + std::pow(10, -elo / 400));
}

double scoreToElo(double score) {
    score = std::clamp(score, 0.001, 0.999);
    return -400 * std::log10(1 / score - 1);
}

struct Summary {
    int pairs = 0;
    double elo = 0;
    double margin = 0;
    double llr = 0;
};

Summary summarize(const Stats &stats, const Options &options) {
    Summary summary;
    double sum = 0;
    for (int i = 0; i < 5; i++) {
        summary.pairs += stats.pairCounts[i];
        sum += stats.pairCounts[i] * i / 4.0; // (i / 4.0) is the value of the bucket: 0, 1/4, 1/2, 3/4, 1
    }
    if (summary.pairs == 0)
        return summary;
    const int n = summary.pairs;
    const double mean = sum / n;

    double variance = 0;
    for (int i = 0; i < 5; i++)
        variance += stats.pairCounts[i] * (i / 4.0 - mean) * (i / 4.0 - mean); // (i / 4.0) is the value of the bucket: 0, 1/4, 1/2, 3/4, 1
    variance /= n;

    const double error = 1.96 * std::sqrt(variance / n); // 1.96 is the 97.5% quantile, giving a 95% confidence interval for two sides
    summary.elo = scoreToElo(mean);
    summary.margin = (scoreToElo(mean + error) - scoreToElo(mean - error)) / 2; // The half-width of the 95% confidence interval for the Elo estimate
    if (variance > 0) {
        const double s0 = eloToScore(options.elo0); // 0 Elo difference corresponds to a 50% expected score
        const double s1 = eloToScore(options.elo1); // 10 Elo difference corresponds to a 51.44% expected score
        summary.llr = n * (s1 - s0) * (2 * mean - s0 - s1) / (2 * variance);
    }
    return summary;
}

void report(const Stats &stats, const Summary &summary) {
    std::printf("pairs %d  A: +%d =%d -%d  pairs [%d %d %d %d %d]  Elo %+.1f ± %.1f  LLR %.2f (±%.2f)  failed %d\n",
                summary.pairs, stats.wins, stats.draws, stats.losses,
                stats.pairCounts[0], stats.pairCounts[1], stats.pairCounts[2], stats.pairCounts[3], stats.pairCounts[4],
                summary.elo, summary.margin, summary.llr, llrBound, stats.failed);
    std::fflush(stdout);
}

Options parseOptions(int argc, char *argv[]) {
    Options options;
    for (int i = 1; i < argc; i += 2) {
        const std::string_view flag(argv[i]);
        if (i + 1 >= argc)
            throw std::invalid_argument("Missing value for " + std::string(flag));
        const std::string value(argv[i + 1]);
        if (flag == "-a")
            options.engineA = value;
        else if (flag == "-b")
            options.engineB = value;
        else if (flag == "-ms")
            options.msPerMove = std::stoi(value);
        else if (flag == "-c")
            options.parallelGames = std::stoi(value);
        else if (flag == "-pairs")
            options.gamePairs = std::stoi(value);
        else if (flag == "-plies")
            options.openingPlies = std::stoi(value);
        else if (flag == "-seed")
            options.seed = std::stoull(value);
        else if (flag == "-elo0")
            options.elo0 = std::stod(value);
        else if (flag == "-elo1")
            options.elo1 = std::stod(value);
        else if (flag == "-games")
            options.gamesFile = value;
        else
            throw std::invalid_argument("Unknown option: " + std::string(flag));
    }
    if (options.engineA.empty() || options.engineB.empty())
        throw std::invalid_argument("Usage: arena -a <engine A> -b <engine B> [-ms 100] [-c 4] [-pairs 1000] [-plies 6] "
                                    "[-seed 1] [-elo0 0] [-elo1 10] [-games FILE]");
    return options;
}

} // namespace

int main(int argc, char *argv[]) {
    Options options;
    try {
        options = parseOptions(argc, argv);
    } catch (const std::exception &error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }

    std::ofstream games;
    if (!options.gamesFile.empty()) {
        games.open(options.gamesFile, std::ios::app);
        if (!games) {
            std::cerr << "Cannot open " << options.gamesFile << std::endl;
            return 1;
        }
    }
    std::printf("A = %s\nB = %s\n%d ms/move, up to %d pairs, %d at a time, H0 %+g Elo, H1 %+g Elo\n",
                options.engineA.c_str(), options.engineB.c_str(), options.msPerMove, options.gamePairs, options.parallelGames, options.elo0, options.elo1);

    std::mutex statsMutex;
    Stats stats;
    Summary summary;
    std::string error;
    std::atomic<int> nextPair = 0;
    std::atomic<bool> stop = false;

    auto worker = [&] {
        try {
            for (int pair = nextPair++; pair < options.gamePairs && !stop; pair = nextPair++) {
                const std::vector<Move> opening = randomOpening(options.openingPlies, options.seed * 1000000 + pair);
                const GameResult aWhite = playGame(options.engineA, options.engineB, opening, options.msPerMove);
                const GameResult aBlack = playGame(options.engineB, options.engineA, opening, options.msPerMove);
                const double scoreAWhite = aWhite.whiteScore;
                const double scoreABlack = 1 - aBlack.whiteScore;

                const std::lock_guard lock(statsMutex);
                stats.addGame(scoreAWhite, aWhite.failed);
                stats.addGame(scoreABlack, aBlack.failed);
                stats.pairCounts[static_cast<int>(std::lround((scoreAWhite + scoreABlack) * 2))]++; // (scoreAWhite + scoreABlack) * 2 gets the bucket index: 0, 1, 2, 3, 4

                // Saves WHITE_SCORE FAILED MOVES for each game
                for (const GameResult *game : {&aWhite, &aBlack}) {
                    if (!games.is_open())
                        continue;
                    games << game->whiteScore << ' ' << game->failed;
                    for (const std::string &move : game->moves)
                        games << ' ' << move;
                    games << '\n';
                }

                summary = summarize(stats, options);
                if (summary.pairs % 10 == 0)
                    report(stats, summary);
                if (std::abs(summary.llr) >= llrBound)
                    stop = true;
            }
        } catch (const std::exception &exception) {
            const std::lock_guard lock(statsMutex);
            error = exception.what();
            stop = true;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < options.parallelGames; i++)
        threads.emplace_back(worker);
    for (std::thread &thread : threads)
        thread.join();

    if (!error.empty()) {
        std::cerr << "Error: " << error << std::endl;
        return 1;
    }
    report(stats, summary);
    if (summary.llr >= llrBound)
        std::printf("H1 accepted: A is stronger than B\n");
    else if (summary.llr <= -llrBound)
        std::printf("H0 accepted: A is not stronger than B\n");
    else
        std::printf("Inconclusive: the pairs ran out before the SPRT decided\n");
    return 0;
}
