#include "engine/MoveOrdering.hpp"
#include <algorithm>

namespace {

// Move order
constexpr int FIRST_MOVE_ORDER = 1 << 30;
constexpr int WIN_ORDER = 1 << 29;
constexpr int CAPTURE_ORDER = 1 << 28;
constexpr int KILLER_ORDER = 1 << 27;
constexpr int MAX_HISTORY = 1 << 20;

/**
 * @brief Calculates the index of a move from its from and to squares
 *
 * @param move The move
 * @return The index, from 0 to 81*81-1
 */
size_t fromTo(const Move &move) {
    return static_cast<size_t>(move.from * 81 + move.to);
}

} // namespace

int &MoveOrdering::historyOf(Side side, const Move &move) {
    return cutoffHistory[static_cast<size_t>(side) * 81 * 81 + fromTo(move)];
}

void MoveOrdering::start(int maxDepth) {
    killers.assign(static_cast<size_t>(maxDepth) + 1, {});
}

std::vector<Child> MoveOrdering::makeChildren(const Position &position, const std::vector<Move> &moves, int ply, Move firstMove, const Move &previous) {
    const auto &killer = killers[static_cast<size_t>(ply)];
    const Move &counterMove = counterMoves[fromTo(previous)];
    const Bitboard captureSquares = position.captureSquares();
    std::vector<Child> children;
    children.reserve(moves.size());
    for (const Move &move : moves) {
        const bool capturing = captureSquares & bit(move.to);
        const bool kingToEdge = (position.king & bit(move.from)) && (tables.edges & bit(move.to));
        Child child{move, capturing || kingToEdge ? position.effects(move) : MoveEffects{}, 0};
        if (move == firstMove)
            child.order = FIRST_MOVE_ORDER;
        else if (child.effects.outcome != Outcome::Ongoing)
            child.order = WIN_ORDER;
        else if (child.effects.captured > 0)
            child.order = CAPTURE_ORDER + child.effects.captured;
        else if (move == killer[0])
            child.order = KILLER_ORDER + 1;
        else if (move == killer[1])
            child.order = KILLER_ORDER;
        else if (move == counterMove)
            child.order = KILLER_ORDER - 1;
        else
            child.order = historyOf(position.sideToMove, move);
        children.push_back(child);
    }
    return children;
}

void MoveOrdering::rememberCutoff(Side side, const Move &move, int depth, int ply, const Move &previous) {
    auto &killer = killers[static_cast<size_t>(ply)];
    if (killer[0] != move) {
        killer[1] = killer[0];
        killer[0] = move;
    }
    counterMoves[fromTo(previous)] = move;
    int &score = historyOf(side, move);
    score = std::min(score + depth * depth, MAX_HISTORY);
}

void MoveOrdering::lowerHistory(Side side, const Move &move, int depth) {
    int &score = historyOf(side, move);
    score = std::max(score - depth * depth, -MAX_HISTORY);
}

const Child &selectNext(std::vector<Child> &children, size_t index) {
    const auto next = children.begin() + static_cast<std::ptrdiff_t>(index);
    std::iter_swap(next, std::max_element(next, children.end(), [](const Child &a, const Child &b) { return a.order < b.order; }));
    return *next;
}

bool isHistoryMove(const Child &child) {
    return child.order < KILLER_ORDER;
}