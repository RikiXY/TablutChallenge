#pragma once

#include "core/Position.hpp"
#include <array>
#include <cstddef>
#include <vector>

struct Child {
    Move move;
    MoveEffects effects;
    int order;
};

class MoveOrdering {
private:
    std::vector<std::array<Move, 2>> killers;
    std::vector<int> cutoffHistory = std::vector<int>(2 * 81 * 81);
    std::vector<Move> counterMoves = std::vector<Move>(81 * 81);

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
     * @brief Calculates the effects of every move and gives it its order
     *
     * @param position The position before the moves
     * @param moves The legal moves
     * @param ply The distance from the root
     * @param firstMove The move to search first
     * @param previous The move that led to the position
     * @return The moves with their effects
     */
    std::vector<Child> makeChildren(const Position &position, const std::vector<Move> &moves, int ply, Move firstMove, const Move &previous);

    /**
     * @brief Remembers a quiet move that caused a cutoff
     *
     * @param side The side that played the move
     * @param move The move
     * @param depth The depth left at the node
     * @param ply The distance from the root
     * @param previous The move that led to the position
     */
    void rememberCutoff(Side side, const Move &move, int depth, int ply, const Move &previous);

    /**
     * @brief Lowers the history of a quiet move that did not cause a cutoff
     *
     * @param side The side that played the move
     * @param move The move
     * @param depth The depth left at the node
     */
    void lowerHistory(Side side, const Move &move, int depth);
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
 * @brief Checks whether a child is ordered by its history or is the countermove
 *
 * @param child The child
 * @return true if the child is not the first move, a win, a capture or a killer
 */
bool isHistoryMove(const Child &child);