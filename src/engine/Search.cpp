#include "engine/Search.hpp"
#include "engine/Evaluation.hpp"
#include <algorithm>
#include <cstdlib>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

struct TimeUp {};

class Searcher {
private:
    Clock::time_point deadline;
    int allowedRepetitions;
    std::vector<Position> path;

public:
    bool canStop = false;
    uint64_t nodes = 0;
    Move rootBest{};

    Searcher(Clock::time_point deadline, const GameHistory &history)
        : deadline(deadline), allowedRepetitions(history.allowedRepetitions), path(history.sinceLastCapture) {}

    int negamax(const Position &position, int depth, int alpha, int beta, int ply, size_t epochStart, Move firstMove) {
        nodes++;
        if (canStop && nodes % 1024 == 0 && Clock::now() >= deadline)
            throw TimeUp{};

        std::vector<Move> moves = position.calculateLegalMoves();
        if (moves.empty())
            return -(WIN - ply); // The opponent already won because this side can't move
        if (depth == 0)
            return evaluate(position);

        const auto first = std::find(moves.begin(), moves.end(), firstMove);
        if (first != moves.end())
            std::iter_swap(moves.begin(), first); // Swap the "firstMove" to the front, so it's searched first

        int best = -WIN - 1; // The worst possible score for the side to move
        for (const Move &move : moves) {
            Position child = position;
            const bool captured = child.apply(move) > 0;
            const size_t childEpoch = captured ? path.size() : epochStart;

            int score;
            if (child.outcome == Outcome::WhiteWins || child.outcome == Outcome::BlackWins)
                score = WIN - (ply + 1);
            else if (isRepetition(child, childEpoch))
                score = 0;
            else {
                path.push_back(child);
                score = -negamax(child, depth - 1, -beta, -alpha, ply + 1, childEpoch, Move{});
                path.pop_back();
            }

            if (score > best) {
                best = score;
                if (ply == 0)
                    rootBest = move;
            }
            alpha = std::max(alpha, best);
            if (alpha >= beta)
                break;
        }
        return best;
    }

private:
    bool isRepetition(const Position &child, size_t epochStart) const {
        const auto seen = std::count(path.begin() + static_cast<std::ptrdiff_t>(epochStart), path.end(), child);
        return seen > allowedRepetitions;
    }
};

} // namespace

SearchResult search(const Position &position, const GameHistory &history, Clock::time_point deadline, int maxDepth) {
    Searcher searcher(deadline, history);
    SearchResult result;

    for (int depth = 1; depth <= maxDepth; depth++) {
        searcher.canStop = depth > 1;
        int score;
        try {
            score = searcher.negamax(position, depth, -WIN - 1, WIN + 1, 0, 0, result.bestMove);
        } catch (const TimeUp &) {
            break;
        }
        result.bestMove = searcher.rootBest;
        result.score = score;
        result.depth = depth;

        if (std::abs(score) >= WIN_BOUND || Clock::now() >= deadline)
            break;
    }
    result.nodes = searcher.nodes;
    return result;
}
