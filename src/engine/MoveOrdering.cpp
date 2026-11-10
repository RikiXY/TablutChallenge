#include "engine/MoveOrdering.hpp"
#include <algorithm>

namespace {

// Move order
constexpr int FIRST_MOVE_ORDER = 1 << 30;
constexpr int WIN_ORDER = 1 << 29;
constexpr int CAPTURE_ORDER = 1 << 28;
constexpr int KILLER_ORDER = 1 << 27;
constexpr int MAX_HISTORY = 1 << 20;

} // namespace

int &MoveOrdering::historyOf(Side side, const Move &move) {
    const int from = move.fromRow * 9 + move.fromCol;
    const int to = move.toRow * 9 + move.toCol;
    return cutoffHistory[static_cast<size_t>((static_cast<int>(side) * 81 + from) * 81 + to)];
}

void MoveOrdering::start(int maxDepth) {
    killers.assign(static_cast<size_t>(maxDepth) + 1, {});
}

std::vector<Child> MoveOrdering::makeChildren(const Position &position, const std::vector<Move> &moves, int ply, Move firstMove) {
    const auto &killer = killers[static_cast<size_t>(ply)];
    std::vector<Child> children;
    children.reserve(moves.size());
    for (const Move &move : moves) {
        Child child{move, position.effects(move), 0};
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
        else
            child.order = historyOf(position.sideToMove, move);
        children.push_back(child);
    }
    return children;
}

void MoveOrdering::rememberCutoff(Side side, const Move &move, int depth, int ply) {
    auto &killer = killers[static_cast<size_t>(ply)];
    if (killer[0] != move) {
        killer[1] = killer[0];
        killer[0] = move;
    }
    int &score = historyOf(side, move);
    score = std::min(score + depth * depth, MAX_HISTORY);
}

const Child &selectNext(std::vector<Child> &children, size_t index) {
    const auto next = children.begin() + static_cast<std::ptrdiff_t>(index);
    std::iter_swap(next, std::max_element(next, children.end(), [](const Child &a, const Child &b) { return a.order < b.order; }));
    return *next;
}

bool isHistoryMove(const Child &child) {
    return child.order < KILLER_ORDER;
}