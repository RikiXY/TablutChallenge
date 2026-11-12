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

void MoveOrdering::addStage(int stage, const Position &position, int ply, Move firstMove, const Move &previous, bool onlyKingMoves,
                            std::vector<TacticalMove> &tactical, std::vector<Move> &moves, std::vector<Child> &children) {
    const auto &killer = killers[static_cast<size_t>(ply)];
    const Move &counterMove = counterMoves[fromTo(previous)];
    switch (stage) {
    case 1: // The first move
        if (position.isLegal(firstMove))
            children.push_back({firstMove, position.effects(firstMove), FIRST_MOVE_ORDER});
        break;
    case 2: // The captures and wins
        position.calculateTacticalMoves(tactical);
        for (const TacticalMove &move : tactical)
            if (move.move != firstMove)
                children.push_back({move.move, move.effects, move.effects.outcome != Outcome::Ongoing ? WIN_ORDER : CAPTURE_ORDER + move.effects.captured});
        break;
    case 3: { // The killers and the countermove
        const std::array<std::pair<Move, int>, 3> candidates = {{{killer[0], KILLER_ORDER + 1}, {killer[1], KILLER_ORDER}, {counterMove, KILLER_ORDER - 1}}};
        for (size_t i = 0; i < candidates.size(); i++) {
            const auto &[move, order] = candidates[i];
            const bool duplicate = move == firstMove || (i >= 1 && move == killer[0]) || (i == 2 && move == killer[1]);
            if (duplicate || !position.isLegal(move))
                continue;
            const MoveEffects effects = position.effects(move);
            if (effects.captured == 0 && effects.outcome == Outcome::Ongoing) // Captures and wins are in stage 2
                children.push_back({move, effects, order});
        }
        break;
    }
    default: { // The other quiet moves
        position.calculateLegalMoves(moves);
        Bitboard tacticalTargets = 0; // Where the stage 2 moves land, to look one up only when a move lands there too
        for (const TacticalMove &move : tactical)
            tacticalTargets |= bit(move.move.to);
        for (const Move &move : moves) {
            if (move == firstMove || move == killer[0] || move == killer[1] || move == counterMove)
                continue;
            if (onlyKingMoves && !(position.king & bit(move.from)))
                continue;
            const auto inStage2 = [&](const TacticalMove &tacticalMove) { return tacticalMove.move == move; };
            if ((tacticalTargets & bit(move.to)) && std::any_of(tactical.begin(), tactical.end(), inStage2))
                continue; // A capture or a win, already added in stage 2
            children.push_back({move, MoveEffects{}, historyOf(position.sideToMove, move)});
        }
        break;
    }
    }
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