#include "engine/Search.hpp"
#include "engine/Evaluation.hpp"
#include "engine/TranspositionTable.hpp"
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <sched.h>
#include <thread>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

struct TimeUp {};

TranspositionTable table(64);

/**
 * @brief Converts a score to store it in the table (the table counts the plies from the node where the score was computed, not from the root)
 *
 * @param score The score
 * @param ply The distance of the node from the root
 * @return The score to store
 */
int toTable(int score, int ply) {
    if (std::abs(score) <= WIN_BOUND) // Not a win or a loss
        return score;
    return score > 0 ? score + ply : score - ply;
}

/**
 * @brief Converts a score read from the table back to counting the plies from the root
 *
 * @param score The stored score
 * @param ply The distance of the node from the root
 * @return The score
 */
int fromTable(int score, int ply) {
    return toTable(score, -ply);
}

/**
 * @brief Checks whether a table result is enough to skip the search
 *
 * @param bound What the stored score means
 * @param score The stored score
 * @param alpha The lower end of the search window
 * @param beta The upper end of the search window
 * @return true for an exact score, a lower bound >= beta or an upper bound <= alpha
 */
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

/**
 * @brief Calculates the bound of a score with respect to the search window
 *
 * @param score The score returned
 * @param alpha The lower end of the search window before the search
 * @param beta The upper end of the search window
 * @return Upper if no move beat alpha, Lower if there was a cutoff, Exact otherwise
 */
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
    const std::atomic<bool> &stop;
    int allowedRepetitions;
    std::vector<uint64_t> path;
    bool useNetwork = isNetworkLoaded();
    std::vector<Accumulator> accumulators;

public:
    bool canStop = false;
    uint64_t nodes = 0;
    Move rootBest{};

    /**
     * @brief Prepares the accumulators for a search of a position
     *
     * @param root The position to search
     * @param maxDepth The maximum depth of the search
     */
    void startAccumulators(const Position &root, int maxDepth) {
        if (!useNetwork)
            return;
        accumulators.resize(static_cast<size_t>(maxDepth) + 1);
        accumulators[0] = makeAccumulator(root);
    }

    /**
     * @param searchDeadline When the search has to stop
     * @param stopSignal Becomes true when the search has to stop right away
     * @param history The game history, where the repetition path starts
     */
    Searcher(Clock::time_point searchDeadline, const std::atomic<bool> &stopSignal, const GameHistory &history)
        : deadline(searchDeadline), stop(stopSignal), allowedRepetitions(history.allowedRepetitions), path(history.sinceLastCapture) {}

    /**
     * @brief Alpha-beta search of a position. Throws TimeUp when the deadline is reached
     *
     * @param position The position to search
     * @param depth How many plies are left to search
     * @param alpha The score the side to move already has guranteed somewhere else
     * @param beta The score the opponent already has guranteed somewhere else, not allowing moves better than that
     * @param ply The distance from the root
     * @param epochStart Where the positions since the last capture start in path
     * @param firstMove The move to search first
     * @return The score from the side to move's point of view
     */
    int negamax(const Position &position, int depth, int alpha, int beta, int ply, size_t epochStart, Move firstMove) {
        nodes++;
        // memory_order_relaxed is enough to just read the value
        if (stop.load(std::memory_order_relaxed) || (canStop && nodes % 1024 == 0 && Clock::now() >= deadline))
            throw TimeUp{};

        const int originalAlpha = alpha;
        const auto entry = depth > 0 && ply > 0 ? table.probe(position.hash) : std::nullopt;
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
            return evaluate(position, defaultWeights, useNetwork ? &accumulators[ply] : nullptr);

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
                if (useNetwork)
                    accumulators[ply + 1] = updateAccumulator(accumulators[ply], position, child);
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
    /**
     * @brief Checks whether a position is a draw by repetition
     *
     * @param child The position after the move
     * @param epochStart Where the positions since the last capture start in path
     * @return true if it was already seen more than allowedRepetitions times
     */
    bool isRepetition(const Position &child, size_t epochStart) const {
        // Counts how many times the child position (its hash) has been seen since the last capture
        const auto seen = std::count(path.begin() + static_cast<std::ptrdiff_t>(epochStart), path.end(), child.hash);
        return seen > allowedRepetitions;
    }
};

/**
 * @brief Iterative deepening until the deadline, maxDepth or a forced win or loss
 *
 * @param searcher The searcher to use
 * @param position The position to search
 * @param firstDepth The depth to start from
 * @param maxDepth The maximum depth
 * @param deadline When to stop
 * @return The best move of the last finished depth, with its score and the depth
 */
SearchResult iterativeDeepening(Searcher &searcher, const Position &position, int firstDepth, int maxDepth, Clock::time_point deadline) {
    SearchResult result;
    searcher.startAccumulators(position, maxDepth);
    for (int depth = firstDepth; depth <= maxDepth; depth++) {
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
    return result;
}

} // namespace

SearchResult search(const Position &position, const GameHistory &history, Clock::time_point deadline, int maxDepth, int threads) {
    table.newSearch();
    std::atomic<bool> stop = false;

    std::vector<Searcher> helpers;
    for (int i = 1; i < threads; i++)
        helpers.emplace_back(deadline, stop, history);
    std::vector<std::thread> workers;
    for (size_t i = 0; i < helpers.size(); i++)
        workers.emplace_back([&, i] { iterativeDeepening(helpers[i], position, 1 + static_cast<int>(i % 2), maxDepth, deadline); });

    Searcher searcher(deadline, stop, history);
    SearchResult result = iterativeDeepening(searcher, position, 1, maxDepth, deadline);
    result.nodes = searcher.nodes;

    stop = true;
    for (std::thread &worker : workers)
        worker.join();
    for (const Searcher &helper : helpers)
        result.nodes += helper.nodes;
    return result;
}

int usableCpus() {
    cpu_set_t set;
    if (sched_getaffinity(0, sizeof(set), &set) != 0)
        return 1;
    return CPU_COUNT(&set);
}

void resizeTranspositionTable(size_t megabytes) {
    table.resize(megabytes);
}
