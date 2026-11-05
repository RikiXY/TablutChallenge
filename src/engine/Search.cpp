#include "engine/Search.hpp"
#include "engine/Evaluation.hpp"
#include "engine/TranspositionTable.hpp"
#include <algorithm>
#include <cstdlib>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

struct TimeUp {};

TranspositionTable table(64);

int toTable(int score, int ply) {
    if (std::abs(score) <= WIN_BOUND) // Not a win or a loss
        return score;
    return score > 0 ? score + ply : score - ply;
}

int fromTable(int score, int ply) {
    return toTable(score, -ply);
}

bool settles(Bound bound, int score, int alpha, int beta) {
    switch (bound) {
    case Bound::Exact:
        return true;
    case Bound::Lower:
        return score >= beta;
    case Bound::Upper:
        return score <= alpha;
    case Bound::None:
        break;
    }
    return false;
}

Bound boundOf(int score, int alpha, int beta) {
    if (score <= alpha)
        return Bound::Upper;
    if (score >= beta)
        return Bound::Lower;
    return Bound::Exact;
}

class Searcher {
private:
    Clock::time_point deadline;
    int allowedRepetitions;
    std::vector<uint64_t> path;

public:
    bool canStop = false;
    uint64_t nodes = 0;
    Move rootBest{};

    Searcher(Clock::time_point searchDeadline, const GameHistory &history)
        : deadline(searchDeadline), allowedRepetitions(history.allowedRepetitions), path(history.sinceLastCapture) {}

    int negamax(const Position &position, int depth, int alpha, int beta, int ply, size_t epochStart, Move firstMove) {
        nodes++;
        if (canStop && nodes % 1024 == 0 && Clock::now() >= deadline)
            throw TimeUp{};

        const int originalAlpha = alpha;
        const TranspositionTable::Entry *entry = depth > 0 && ply > 0 ? table.probe(position.hash) : nullptr;
        if (entry) {
            const int score = fromTable(entry->score, ply);
            if (entry->depth >= depth && settles(entry->bound, score, alpha, beta))
                return score;
            firstMove = entry->move();
        }

        std::vector<Move> moves = position.calculateLegalMoves(); // Generate all branches
        if (moves.empty())
            return -(WIN - ply); // The opponent already won because this side can't move
                                 // ! -ply indica che l'algoritmo preferisce perdere il più tardi possibile
        if (depth == 0)
            return evaluate(position);

        const auto first = std::find(moves.begin(), moves.end(), firstMove);
        if (first != moves.end())
            std::iter_swap(moves.begin(), first); // Swap the "firstMove" to the front, so it's searched first

        int best = -WIN - 1; // ! The worst possible score for the side to move (like -inf in minimax)
                             // ! perché non direttamente -WIN
        Move bestMove{};
        for (const Move &move : moves) {
            Position child = position;
            const bool captured = child.apply(move) > 0;
            const size_t childEpoch = captured ? path.size() : epochStart; // ?

            int score;
            if (child.outcome == Outcome::WhiteWins || child.outcome == Outcome::BlackWins)
                score = WIN - (ply + 1);
            else if (isRepetition(child, childEpoch))
                score = 0;
            else {
                path.push_back(child.hash);
                score = -negamax(child, depth - 1, -beta, -alpha, ply + 1, childEpoch, Move{});
                path.pop_back();
            }

            if (score > best) {
                best = score;
                bestMove = move;
                if (ply == 0)
                    rootBest = move;
            }
            alpha = std::max(alpha, best);
            if (alpha >= beta)
                break;
        }

        table.store(position.hash, bestMove, toTable(best, ply), depth, boundOf(best, originalAlpha, beta));
        return best;
    }

private:
    bool isRepetition(const Position &child, size_t epochStart) const {
        // Counts how many times the child position (its hash) has been seen since the last capture
        const auto seen = std::count(path.begin() + static_cast<std::ptrdiff_t>(epochStart), path.end(), child.hash);
        return seen > allowedRepetitions;
    }
};

} // namespace

SearchResult search(const Position &position, const GameHistory &history, Clock::time_point deadline, int maxDepth) {
    Searcher searcher(deadline, history);
    SearchResult result;
    table.newSearch();

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

void resizeTranspositionTable(size_t megabytes) {
    table.resize(megabytes);
}
