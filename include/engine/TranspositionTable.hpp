#pragma once

#include "core/Board.hpp"
#include <cstddef>
#include <cstdint>
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
    std::vector<Entry> entries;
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
     * @return The entry or nullptr if the position isn't stored
     */
    const Entry *probe(uint64_t key) const;

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
