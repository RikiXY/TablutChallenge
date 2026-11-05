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

        Move move() const;
    };

private:
    std::vector<Entry> entries;
    uint8_t generation = 0;

public:
    explicit TranspositionTable(size_t megabytes);

    void resize(size_t megabytes);
    void newSearch();
    const Entry *probe(uint64_t key) const;
    void store(uint64_t key, const Move &move, int score, int depth, Bound bound);
};
