#pragma once

#include "core/Bitboard.hpp"
#include "core/Board.hpp"
#include <cstdint>
#include <vector>

enum class Side : uint8_t { White,
                            Black };
enum class Outcome : uint8_t { Ongoing,
                               WhiteWins,
                               BlackWins,
                               Draw };

struct MoveEffects {
    uint8_t capturedDirections = 0; // Bit d = 1 if the neighbour in directions[d] gets captured
    uint8_t captured = 0;
    Outcome outcome = Outcome::Ongoing;
    bool operator==(const MoveEffects &) const = default;
};

struct TacticalMove {
    Move move;
    MoveEffects effects;
};

struct Position {
    Bitboard white = 0; // The white pawns exluding the king
    Bitboard black = 0; // The black pawns
    Bitboard king = 0;  // The king
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
     * @brief Reads the piece on a square
     *
     * @param square The square
     * @return What stands on it
     */
    Piece at(int square) const;

    /**
     * @brief Replaces a piece on a square
     *
     * @param square The square
     * @param piece The piece to replace with
     */
    void put(int square, Piece piece);

    /**
     * @brief Finds the square of the king
     *
     * @return row * 9 + col, or -1 if there is no king
     */
    int kingSquare() const;

    /**
     * @brief Generates the legal moves for every piece of the side to move
     *
     * @param moves Where to write the moves
     */
    void calculateLegalMoves(std::vector<Move> &moves) const;

    /**
     * @brief Checks whether a move is legal
     *
     * @param move The move
     * @return true if the side to move can play it
     */
    bool isLegal(const Move &move) const;

    /**
     * @brief Checks whether the side to move has at least one legal move
     *
     * @return true if there is at least one legal move
     */
    bool hasLegalMove() const;

    /**
     * @brief Generates only the legal moves that capture or win, with their effects
     *
     * @param moves Where to write the moves
     */
    void calculateTacticalMoves(std::vector<TacticalMove> &moves) const;

    /**
     * @brief Plays a move: moves the piece, removes the captured pawns and updates the outcome, the side to move and the hash
     *
     * @param move The move to play (legal)
     * @return How many pawns were captured
     */
    int apply(const Move &move);

    /**
     * @brief Finds the squares where a move can capture or where the king can escape
     *
     * @return The squares where a move can have an effect
     */
    Bitboard captureSquares() const;

    /**
     * @brief Checks whether the side to move can end the game with its next move, without generating the moves
     *
     * @return true if there is a winning move
     */
    bool hasWinningMove() const;

    /**
     * @brief Calculates what a move would capture and whether it would end the game
     *
     * @param move The move (legal)
     * @return The effects of the move
     */
    MoveEffects effects(const Move &move) const;

    /**
     * @brief Plays a move whose effects are already known
     *
     * @param move The move to play (legal)
     * @param effects The effects of the move
     * @return How many pawns were captured
     */
    int apply(const Move &move, const MoveEffects &effects);

    /**
     * @brief Calculates the hash after a move that doesn't capture anything, without playing it
     *
     * @param move The move (legal, capturing nothing)
     * @return The hash of the position after the move
     */
    uint64_t hashAfterQuietMove(const Move &move) const;

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

/**
 * @brief Prints the board on the terminal
 *
 * @param position The position to print
 */
void printBoard(const Position &position);