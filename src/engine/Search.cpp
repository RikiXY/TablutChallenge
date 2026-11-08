#include "engine/Search.hpp"
#include "engine/Evaluation.hpp"
#include "engine/MoveOrdering.hpp"
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

class Searcher {
private:
    Clock::time_point deadline;
    const std::atomic<bool> &stop;
    int allowedRepetitions;
    std::vector<uint64_t> path;
    bool useNetwork = isNetworkLoaded();
    std::vector<Accumulator> accumulators;
    MoveOrdering ordering;

public:
    bool canStop = false;
    uint64_t nodes = 0;
    Move rootBest{};

    /**
     * @brief Prepares the accumulators and the killers for a search of a position
     *
     * @param root The position to search
     * @param maxDepth The maximum depth of the search
     */
    void start(const Position &root, int maxDepth) {
        ordering.start(maxDepth);
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
        const bool pvNode = beta - alpha > 1;
        const auto entry = depth > 0 && ply > 0 ? table.probe(position.hash) : std::nullopt;
        if (entry) {
            const int score = fromTable(entry->score, ply);
            if (!pvNode && entry->depth >= depth && settles(entry->bound, score, alpha, beta))
                return score;
            firstMove = entry->move();
        }

        std::vector<Move> moves = position.calculateLegalMoves(); // Generate all branches
        if (moves.empty())
            return -(WIN - ply); // The opponent already won because this side can't move
                                 // ! -ply indica che l'algoritmo preferisce perdere il più tardi possibile
        if (depth == 0)
            return evaluate(position, defaultWeights, useNetwork ? &accumulators[ply] : nullptr);

        std::vector<Child> children = ordering.makeChildren(position, moves, ply, firstMove);

        int best = -WIN - 1; // ! The worst possible score for the side to move (like -inf in minimax)
        Move bestMove{};
        for (size_t i = 0; i < children.size(); i++) {
            const Child &next = selectNext(children, i);
            const Move &move = next.move;
            const Position &child = next.position;
            const bool captured = next.captured > 0;
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
                if (i == 0)
                    score = -negamax(child, depth - 1, -beta, -alpha, ply + 1, childEpoch, Move{});
                else {
                    score = -negamax(child, depth - 1, -alpha - 1, -alpha, ply + 1, childEpoch, Move{});
                    if (alpha < score && score < beta)
                        score = -negamax(child, depth - 1, -beta, -alpha, ply + 1, childEpoch, Move{});
                }
                path.pop_back();
            }

            if (score > best) {
                best = score;
                bestMove = move;
                if (ply == 0)
                    rootBest = move;
            }
            alpha = std::max(alpha, best);
            if (alpha >= beta) {
                if (!captured)
                    ordering.rememberCutoff(position.sideToMove, move, depth, ply);
                break;
            }
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

constexpr int ASPIRATION_DEPTH = 4;
constexpr int ASPIRATION_WINDOW = 30;

/**
 * @brief Searches the root with a narrow window around the previous score, widening it until the score falls inside
 *
 * @param searcher The searcher to use
 * @param position The position to search
 * @param depth The depth to search
 * @param previous The result of the previous depth
 * @return The exact score of the position
 */
int aspirationSearch(Searcher &searcher, const Position &position, int depth, const SearchResult &previous) {
    int alpha = -WIN - 1;
    int beta = WIN + 1;
    int window = ASPIRATION_WINDOW;
    if (depth >= ASPIRATION_DEPTH && previous.depth > 0 && std::abs(previous.score) < WIN_BOUND) {
        alpha = previous.score - window;
        beta = previous.score + window;
    }
    Move firstMove = previous.bestMove;
    while (true) {
        const int score = searcher.negamax(position, depth, alpha, beta, 0, 0, firstMove);
        if (score <= alpha)
            alpha = std::max(score - window, -WIN - 1);
        else if (score >= beta)
            beta = std::min(score + window, WIN + 1);
        else
            return score;
        firstMove = searcher.rootBest;
        window *= 2;
    }
}

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
    searcher.start(position, maxDepth);
    for (int depth = firstDepth; depth <= maxDepth; depth++) {
        searcher.canStop = depth > 1;
        int score;
        try {
            score = aspirationSearch(searcher, position, depth, result);
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
