#pragma once

#include "core/Board.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

enum class Bound : uint8_t { None,
                             Upper,
                             Lower,
                             Exact };

class TranspositionTable {
public:
    struct Entry {
        uint64_t key = 0;
        int16_t score = 0;
        uint8_t depth = 0;
        Bound bound = Bound::None;
        uint8_t generation = 0;
        uint8_t from = 0, to = 0; // The best move written as row * 9 + col

        /**
         * @return The best move
         */
        Move move() const;
    };

private:
    // Shared by the search threads
    // keyXorData is key ^ data
    struct Slot {
        uint64_t keyXorData = 0;
        uint64_t data = 0;
    };

    std::vector<Slot> slots;
    uint8_t generation = 0;

public:
    /**
     * @brief Creates an empty table
     *
     * @param megabytes The size of the table in MiB
     */
    explicit TranspositionTable(size_t megabytes);

    /**
     * @brief Reallocates the table with a new size and clears it
     *
     * @param megabytes The new size of the table in MiB
     */
    void resize(size_t megabytes);

    /**
     * @brief Starts a new search, so the entries of the old ones get replaced first
     */
    void newSearch();

    /**
     * @brief Looks up a position
     *
     * @param key The position hash
     * @return A copy of the entry, nothing if the position isn't stored
     */
    std::optional<Entry> probe(uint64_t key) const;

    /**
     * @brief Saves a search result. Replaces the slot unless it holds another position from this same search with a deeper search result
     *
     * @param key The position hash
     * @param move The best move found
     * @param score The score calculated
     * @param depth The depth searched
     * @param bound Whether the score is exact or only a bound
     */
    void store(uint64_t key, const Move &move, int score, int depth, Bound bound);
};

/**
 * @brief Converts a score to store it in the table (the table counts the plies from the node where the score was computed, not from the root)
 *
 * @param score The score
 * @param ply The distance of the node from the root
 * @return The score to store
 */
int toTable(int score, int ply);

/**
 * @brief Converts a score read from the table back to counting the plies from the root
 *
 * @param score The stored score
 * @param ply The distance of the node from the root
 * @return The score
 */
int fromTable(int score, int ply);

/**
 * @brief Checks whether a table result is enough to skip the search
 *
 * @param bound What the stored score means
 * @param score The stored score
 * @param alpha The lower end of the search window
 * @param beta The upper end of the search window
 * @return true for an exact score, a lower bound >= beta or an upper bound <= alpha
 */
bool settles(Bound bound, int score, int alpha, int beta);

/**
 * @brief Calculates the bound of a score with respect to the search window
 *
 * @param score The score returned
 * @param alpha The lower end of the search window before the search
 * @param beta The upper end of the search window
 * @return Upper if no move beat alpha, Lower if there was a cutoff, Exact otherwise
 */
Bound boundOf(int score, int alpha, int beta);
