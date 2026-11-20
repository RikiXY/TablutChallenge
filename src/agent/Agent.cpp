#include "agent/Agent.hpp"
#include "core/Translator.hpp"
#include "engine/Search.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

Agent::Agent(int timeoutSeconds, int searchThreads) : timeout(timeoutSeconds), threads(searchThreads) {}

Move Agent::chooseMove(const Position &position, const GameHistory &history, std::chrono::steady_clock::time_point startingTime) {
    if (!position.hasLegalMove())
        throw NoLegalMoves{};

    if (pending.valid())
        pending.wait();

    // The search budget is the timeout minus a reserve of 3% of it for safety, kept between 350 ms and 2 s
    const int timeoutMs = timeout * 1000;
    const int reserveMs = std::clamp(timeout * 30, 350, 2000);
    const int budgetMs = std::max(timeoutMs - reserveMs, 50);
    const auto deadline = startingTime + std::chrono::milliseconds(budgetMs);

    pending = std::async(std::launch::async, [position, history, deadline, searchThreads = threads] {
        return search(position, history, deadline, 64, searchThreads);
    });
    const auto grace = std::chrono::milliseconds(std::min(200, budgetMs / 4));

    Move move{};
    std::string details;
    if (pending.wait_until(deadline + grace) == std::future_status::ready) {
        const SearchResult result = pending.get();
        move = result.bestMove;
        details = "\tdepth " + std::to_string(result.depth) + ", score " + std::to_string(result.score) + ", " + std::to_string(result.nodes) + " nodes";
    } else {
        move = completedMove();
        stopSearch();
        details = "\tWatchdog: the search didn't stop in time, sent the best move of the last finished depth";
    }

    if (!position.isLegal(move)) {
        std::vector<Move> legal;
        position.calculateLegalMoves(legal);
        move = legal.front();
        details += "\n\tThe move found wasn't legal, sent the first legal move instead";
    }

    std::cout << "Chosen move: \n\tfrom " << convertToCoord(move.from)
              << " to " << convertToCoord(move.to) << std::endl;
    std::cout << details << std::endl;
    return move;
}
