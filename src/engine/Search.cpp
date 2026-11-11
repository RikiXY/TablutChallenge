#include "engine/Search.hpp"
#include "engine/Evaluation.hpp"
#include "engine/MoveOrdering.hpp"
#include "engine/TranspositionTable.hpp"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <sched.h>
#include <thread>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

struct TimeUp {};

TranspositionTable table(64);

constexpr double LMR_DIVISOR = 150;
constexpr int LMP_FACTOR = 3;
constexpr int QUIESCENCE_DEPTH = 4;
constexpr int RFP_MARGIN = 50;
constexpr int DAMP = 200;

class Searcher {
private:
    Clock::time_point deadline;
    const std::atomic<bool> &stop;
    bool exact;
    int allowedRepetitions;
    std::vector<uint64_t> path;
    bool useNetwork = isNetworkLoaded();
    std::vector<Accumulator> accumulators;
    MoveOrdering ordering;
    std::vector<Move> previousMoves;

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
        previousMoves.assign(static_cast<size_t>(maxDepth) + 1, Move{});
        if (!useNetwork)
            return;
        accumulators.resize(static_cast<size_t>(maxDepth + QUIESCENCE_DEPTH) + 1);
        accumulators[0] = makeAccumulator(root);
    }

    /**
     * @param searchDeadline When the search has to stop
     * @param stopSignal Becomes true when the search has to stop right away
     * @param history The game history, where the repetition path starts
     * @param exactSearch true to search every move at full depth and completely
     */
    Searcher(Clock::time_point searchDeadline, const std::atomic<bool> &stopSignal, const GameHistory &history, bool exactSearch)
        : deadline(searchDeadline), stop(stopSignal), exact(exactSearch), allowedRepetitions(history.allowedRepetitions), path(history.sinceLastCapture) {}

    /**
     * @brief Alpha-beta search of a position. Throws TimeUp when the deadline is reached
     *
     * @param position The position to search
     * @param depth How many plies are left to search
     * @param alpha The score the side to move already has guaranteed somewhere else
     * @param beta The score the opponent already has guaranteed somewhere else, not allowing moves better than that
     * @param ply The distance from the root
     * @param epochStart Where the positions since the last capture start in path
     * @param firstMove The move to search first
     * @return The score from the side to move's point of view
     */
    int negamax(const Position &position, int depth, int alpha, int beta, int ply, size_t epochStart, Move firstMove) {
        nodes++;
        checkTime();

        const int originalAlpha = alpha;
        const bool pvNode = beta - alpha > 1;
        const auto entry = depth > 0 && ply > 0 ? table.probe(position.hash) : std::nullopt;
        if (entry) {
            const int score = fromTable(entry->score, ply);
            if (!pvNode && entry->depth >= depth && settles(entry->bound, score, alpha, beta))
                return score;
            firstMove = entry->move();
        }

        if (depth == 0 && position.hasLegalMove()) {
            if (exact)
                return staticScore(position, ply, epochStart);
            return quiescence(position, alpha, beta, ply, QUIESCENCE_DEPTH, epochStart);
        }

        if (!exact && depth >= 4 && firstMove == Move{})
            depth--;

        const bool canSkipLate = !exact && !pvNode && ply > 0 && depth <= 3;

        if (canSkipLate && std::abs(beta) < WIN_BOUND && position.hasLegalMove()) {
            const int score = staticScore(position, ply, epochStart);
            const bool escapeThreat = position.sideToMove == Side::Black && kingOpenRays(position) > 0;
            if (score - RFP_MARGIN * depth >= beta && !escapeThreat)
                return score;
        }

        std::vector<Move> moves = position.calculateLegalMoves(); // Generate all branches
        if (moves.empty())
            return -(WIN - ply); // The opponent already won because this side can't move
                                 // ! -ply indica che l'algoritmo preferisce perdere il più tardi possibile

        const Move &previous = previousMoves[static_cast<size_t>(ply)];
        std::vector<Child> children = ordering.makeChildren(position, moves, ply, firstMove, previous);

        const size_t lmpMoves = static_cast<size_t>(LMP_FACTOR * depth * depth);

        int best = -WIN - 1; // ! The worst possible score for the side to move (like -inf in minimax)
        Move bestMove{};
        for (size_t i = 0; i < children.size(); i++) {
            const Child &next = selectNext(children, i);
            const Move &move = next.move;
            const bool captured = next.effects.captured > 0;

            const bool notLosing = best > -WIN_BOUND;
            const bool lateQuiet = isHistoryMove(next) && !isKingMove(position, next);
            if (canSkipLate && i >= lmpMoves && notLosing && lateQuiet) {
                keepOnlyKingMoves(position, children, i + 1);
                continue;
            }

            int score;
            if (next.effects.outcome != Outcome::Ongoing)
                score = WIN - (ply + 1);
            else {
                Position child = position;
                child.apply(move, next.effects);
                const size_t childEpoch = captured ? path.size() : epochStart;

                // Searches the child and returns its score from this side's point of view with the window from this side's point of view
                auto searchChild = [&](int childDepth, int low, int high) { return -negamax(child, childDepth, -high, -low, ply + 1, childEpoch, Move{}); };

                if (isRepetition(child.hash, childEpoch))
                    score = 0;
                else {
                    path.push_back(child.hash);
                    if (useNetwork)
                        accumulators[ply + 1] = updateAccumulator(accumulators[ply], position, child);
                    previousMoves[static_cast<size_t>(ply) + 1] = move;
                    if (i == 0)
                        score = searchChild(depth - 1, alpha, beta);
                    else {
                        const int reduction = lateMoveReduction(depth, i, next);
                        score = searchChild(depth - 1 - reduction, alpha, alpha + 1); // Null window, maybe reduced
                        if (reduction > 0 && score > alpha)
                            score = searchChild(depth - 1, alpha, alpha + 1); // Beat alpha while reduced so verify at full depth
                        if (alpha < score && score < beta)
                            score = searchChild(depth - 1, alpha, beta); // Inside the window so calculate the exact value
                    }
                    path.pop_back();
                }
            }

            if (ply == 0 && (i == 0 || score > alpha))
                rootBest = move;
            if (score > best) {
                best = score;
                bestMove = move;
            }
            alpha = std::max(alpha, best);
            if (alpha >= beta) {
                if (!captured) {
                    ordering.rememberCutoff(position.sideToMove, move, depth, ply, previous);
                    for (size_t j = 0; j < i; j++)
                        if (children[j].effects.captured == 0)
                            ordering.lowerHistory(position.sideToMove, children[j].move, depth);
                }
                break;
            }
        }

        table.store(position.hash, bestMove, toTable(best, ply), depth, boundOf(best, originalAlpha, beta));
        return best;
    }

private:
    /**
     * @brief Searches only captures, wins, and king escape threats after the normal depth
     *
     * @param position The position to search
     * @param alpha The lower end of the search window
     * @param beta The upper end of the search window
     * @param ply The distance from the root
     * @param depth How many plies of quiescence are left
     * @param epochStart Where the positions since the last capture start in path
     * @return The score from the side to move's point of view
     */
    int quiescence(const Position &position, int alpha, int beta, int ply, int depth, size_t epochStart) {
        const std::vector<Move> moves = position.calculateTacticalMoves();
        for (const Move &move : moves)
            if (position.effects(move).outcome != Outcome::Ongoing)
                return WIN - (ply + 1);

        if (position.sideToMove == Side::Black) {
            const int rays = kingOpenRays(position);
            if (rays > 0)
                return defendEscape(position, alpha, beta, ply, depth, epochStart, rays);
        }

        int best = staticScore(position, ply, epochStart);
        if (depth == 0 || best >= beta)
            return best;
        alpha = std::max(alpha, best);

        std::vector<Move> candidates = moves;
        if (position.sideToMove == Side::White)
            addQuietKingMoves(position, candidates);

        for (const Move &move : candidates) {
            const MoveEffects effects = position.effects(move);
            Position child = position;
            child.apply(move, effects);
            if (effects.captured == 0 && kingOpenRays(child) == 0)
                continue;

            const int score = searchQuiescenceMove(position, child, effects.captured > 0, alpha, beta, ply, depth, epochStart);
            best = std::max(best, score);
            alpha = std::max(alpha, best);
            if (alpha >= beta)
                break;
        }
        return best;
    }

    /**
     * @brief Quiescence for Black when the king has an open line
     *
     * @param position The position (Black to move) with no immediate win
     * @param alpha The lower end of the search window
     * @param beta The upper end of the search window
     * @param ply The distance from the root
     * @param depth How many plies of quiescence are left
     * @param epochStart Where the positions since the last capture start in path
     * @param rays How many open lines the king has
     * @return The score from Black's point of view
     */
    int defendEscape(const Position &position, int alpha, int beta, int ply, int depth, size_t epochStart, int rays) {
        if (depth == 0 && rays == 1)
            return staticScore(position, ply, epochStart);

        int kingRow = 0;
        int kingCol = 0;
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                if (position.board[row][col] == Piece::King) {
                    kingRow = row;
                    kingCol = col;
                }
            }
        }

        int best = -(WIN - (ply + 2)); // +2 because the king can escape in one move and then Black can capture it in one move
        for (const Move &move : position.calculateLegalMoves()) {
            int score;
            if (isRepetition(position.hashAfterQuietMove(move), epochStart) && position.effects(move).captured == 0)
                score = 0;
            else if (rays == 1 && depth > 0 && (move.toRow == kingRow || move.toCol == kingCol)) {
                const MoveEffects effects = position.effects(move);
                Position child = position;
                child.apply(move, effects);
                if (kingOpenRays(child) > 0)
                    continue;
                score = searchQuiescenceMove(position, child, effects.captured > 0, alpha, beta, ply, depth, epochStart);
            } else {
                continue;
            }

            best = std::max(best, score);
            alpha = std::max(alpha, best);
            if (alpha >= beta)
                break;
        }
        return best;
    }

    /**
     * @brief Searches a quiescence child
     *
     * @param position The position before the move
     * @param child The position after the move
     * @param captured Whether the move captured
     * @param alpha The lower end of the window
     * @param beta The upper end of the window
     * @param ply The distance of position from the root
     * @param depth How many plies of quiescence are left at position
     * @param epochStart Where the positions since the last capture start in path
     * @return The score from the point of view of the side that moved
     */
    int searchQuiescenceMove(const Position &position, const Position &child, bool captured, int alpha, int beta, int ply, int depth, size_t epochStart) {
        nodes++;
        checkTime();
        const size_t childEpoch = captured ? path.size() : epochStart;
        if (isRepetition(child.hash, childEpoch))
            return 0;
        if (!child.hasLegalMove())
            return WIN - (ply + 1);

        path.push_back(child.hash);
        if (useNetwork)
            accumulators[ply + 1] = updateAccumulator(accumulators[ply], position, child);
        const int score = -quiescence(child, -beta, -alpha, ply + 1, depth - 1, childEpoch);
        path.pop_back();
        return score;
    }

    /**
     * @brief Adds the king moves that don't capture
     *
     * @param position The position, White to move
     * @param moves Where to add the moves
     */
    static void addQuietKingMoves(const Position &position, std::vector<Move> &moves) {
        for (int row = 0; row < 9; row++) {
            for (int col = 0; col < 9; col++) {
                if (position.board[row][col] != Piece::King)
                    continue;
                for (const auto &[dRow, dCol] : directions) {
                    int r = row + dRow;
                    int c = col + dCol;
                    while (isOnBoard(r, c) && position.board[r][c] == Piece::Empty && !isThrone(r, c) && !isCamp(r, c)) {
                        const Move move{row, col, r, c};
                        if (position.effects(move).captured == 0)
                            moves.push_back(move);
                        r += dRow;
                        c += dCol;
                    }
                }
                return;
            }
        }
    }

    /**
     * @brief Throws TimeUp if the search has to stop
     */
    void checkTime() const {
        // memory_order_relaxed is enough to just read the value
        if (stop.load(std::memory_order_relaxed) || (canStop && nodes % 1024 == 0 && Clock::now() >= deadline))
            throw TimeUp{};
    }

    /**
     * @brief Evaluates a position, moving the score towards a draw the longer the game goes without a capture
     *
     * @param position The position
     * @param ply The distance from the root
     * @param epochStart Where the positions since the last capture start in path
     * @return The score from the side to move's point of view
     */
    int staticScore(const Position &position, int ply, size_t epochStart) const {
        const int score = evaluate(position, defaultWeights, useNetwork ? &accumulators[static_cast<size_t>(ply)] : nullptr);
        if (exact)
            return score;
        const int quietPlies = std::clamp(static_cast<int>(path.size() - epochStart) - 1, 0, DAMP / 2);
        return score * (DAMP - quietPlies) / DAMP;
    }

    /**
     * @brief Calculates how much to reduce a move
     *
     * @param depth The depth left at the node
     * @param index The position of the move in the order
     * @param child The move
     * @return log(depth) * log(index) * 100 / LMR_DIVISOR clamped between 1 and depth - 2, 0 if the move isn't reduced
     */
    int lateMoveReduction(int depth, size_t index, const Child &child) const {
        if (exact || depth < 3 || index < 4 || !isHistoryMove(child))
            return 0;
        return std::clamp(static_cast<int>(std::log(depth) * std::log(static_cast<double>(index)) * 100 / LMR_DIVISOR), 1, depth - 2);
    }

    /**
     * @brief Checks whether a move is a king move
     *
     * @param position The position before the move
     * @param child The child
     * @return true if the king moves
     */
    static bool isKingMove(const Position &position, const Child &child) {
        return position.board[child.move.fromRow][child.move.fromCol] == Piece::King;
    }

    /**
     * @brief Removes the non-king moves from an index until the end
     *
     * @param position The position before the moves
     * @param children The children
     * @param from The first child that can be removed
     */
    static void keepOnlyKingMoves(const Position &position, std::vector<Child> &children, size_t from) {
        const auto first = children.begin() + static_cast<std::ptrdiff_t>(from);
        const auto kept = std::remove_if(first, children.end(), [&](const Child &child) { return !isKingMove(position, child); });
        children.erase(kept, children.end());
    }

    /**
     * @brief Checks whether a position is a draw by repetition
     *
     * @param hash The hash of the position after the move
     * @param epochStart Where the positions since the last capture start in path
     * @return true if it was already seen more than allowedRepetitions times
     */
    bool isRepetition(uint64_t hash, size_t epochStart) const {
        // Counts how many times the child position (its hash) has been seen since the last capture
        const auto seen = std::count(path.begin() + static_cast<std::ptrdiff_t>(epochStart), path.end(), hash);
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
        window *= 2;
        if (score <= alpha)
            alpha = std::max(score - window, -WIN - 1);
        else if (score >= beta)
            beta = std::min(score + window, WIN + 1);
        else
            return score;
        firstMove = searcher.rootBest;
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
 * @return The best move, with the score and the depth of the last finished depth
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
            if (searcher.rootBest != Move{})
                result.bestMove = searcher.rootBest;
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

SearchResult search(const Position &position, const GameHistory &history, Clock::time_point deadline, int maxDepth, int threads, bool exact) {
    table.newSearch();
    std::atomic<bool> stop = false;

    std::vector<Searcher> helpers;
    for (int i = 1; i < threads; i++)
        helpers.emplace_back(deadline, stop, history, exact);
    std::vector<std::thread> workers;
    for (size_t i = 0; i < helpers.size(); i++)
        workers.emplace_back([&, i] { iterativeDeepening(helpers[i], position, 1 + static_cast<int>(i % 2), maxDepth, deadline); });

    Searcher searcher(deadline, stop, history, exact);
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
