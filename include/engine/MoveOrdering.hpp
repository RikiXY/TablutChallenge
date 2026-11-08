#pragma once

#include "core/Position.hpp"
#include <array>
#include <cstddef>
#include <vector>

struct Child {
    Move move;
    Position position;
    int captured;
    int order;
};

class MoveOrdering {
private:
    std::vector<std::array<Move, 2>> killers;
    std::vector<int> cutoffHistory = std::vector<int>(2 * 81 * 81);

    /**
     * @brief Finds the history of a move
     *
     * @param side The side that plays the move
     * @param move The move
     * @return The history score
     */
    int &historyOf(Side side, const Move &move);

public:
    /**
     * @brief Prepares the killers for a new search
     *
     * @param maxDepth The maximum depth of the search
     */
    void start(int maxDepth);

    /**
     * @brief Plays every move and gives it its order
     *
     * @param position The position before the moves
     * @param moves The legal moves
     * @param ply The distance from the root
     * @param firstMove The move to search first
     * @return The moves with their positions
     */
    std::vector<Child> makeChildren(const Position &position, const std::vector<Move> &moves, int ply, Move firstMove);

    /**
     * @brief Remembers a quiet move that caused a cutoff
     *
     * @param side The side that played the move
     * @param move The move
     * @param depth The depth left at the node
     * @param ply The distance from the root
     */
    void rememberCutoff(Side side, const Move &move, int depth, int ply);
};

/**
 * @brief Moves the best of the children not searched yet to index
 *
 * @param children The children
 * @param index How many children were already searched
 * @return The child to search next
 */
const Child &selectNext(std::vector<Child> &children, size_t index);

/**
 * @brief Checks whether a child is ordered only by its history
 *
 * @param child The child
 * @return true if the child is ordered only by its history
 */
bool isHistoryMove(const Child &child);