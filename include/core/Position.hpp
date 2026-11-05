#pragma once

#include "core/Board.hpp"
#include <cstdint>
#include <vector>

enum class Side : uint8_t { White,
                            Black };
enum class Outcome : uint8_t { Ongoing,
                               WhiteWins,
                               BlackWins,
                               Draw };

struct Position {
    BoardMatrix board{};
    Side sideToMove = Side::White;
    Outcome outcome = Outcome::Ongoing;
    uint64_t hash = 0;

    /**
     * @brief Creates the starting position
     *
     * @return The starting position with White to move
     */
    static Position start(); // Static method: callable without an instance of the struct

    /**
     * @brief Generates the legal moves for every piece of the side to move, without modifying the position
     *
     * @return The legal moves, empty if the game is over or the side to move is stuck
     */
    std::vector<Move> calculateLegalMoves() const;

    /**
     * @brief Plays a move: moves the piece, removes the captured pawns and updates the outcome, the side to move and the hash
     *
     * @param move The move to play (legal)
     * @return How many pawns were captured
     */
    int apply(const Move &move);

    /**
     * @brief Recomputes the hash from scratch. Necessary after changing board or sideToMove by hand
     */
    void computeHash();

    bool operator==(const Position &) const = default; // Automatically compare all members for equality
};

/**
 * @brief Counts the positions reachable in exactly depth moves
 *
 * @param position The position to start from
 * @param depth How many moves to play, at least 1
 * @return The number of positions, without counting games that end before the last move
 */
uint64_t perft(const Position &position, int depth);