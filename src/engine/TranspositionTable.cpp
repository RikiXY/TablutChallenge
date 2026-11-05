#include "engine/TranspositionTable.hpp"

Move TranspositionTable::Entry::move() const {
    return {from / 9, from % 9, to / 9, to % 9};
}

TranspositionTable::TranspositionTable(size_t megabytes) {
    resize(megabytes);
}

void TranspositionTable::resize(size_t megabytes) {
    size_t count = 1;
    while (count * 2 * sizeof(Entry) <= megabytes * 1024 * 1024)
        count *= 2;
    entries.assign(count, Entry{});
}

void TranspositionTable::newSearch() {
    generation++;
}

const TranspositionTable::Entry *TranspositionTable::probe(uint64_t key) const {
    const Entry &entry = entries[key & (entries.size() - 1)]; // key & (entries.size() - 1) = hash % entries.size() but faster because size is a power of 2
    return entry.bound != Bound::None && entry.key == key ? &entry : nullptr;
}

// Replace the slot unless it holds a result for another position from this same search with a deeper search result
void TranspositionTable::store(uint64_t key, const Move &move, int score, int depth, Bound bound) {
    Entry &entry = entries[key & (entries.size() - 1)]; // key & (entries.size() - 1) = hash % entries.size() but faster because size is a power of 2
    if (entry.key != key && entry.generation == generation && entry.depth > depth)
        return;
    entry = {key, static_cast<int16_t>(score), static_cast<uint8_t>(depth), bound, generation,
             static_cast<uint8_t>(move.fromRow * 9 + move.fromCol), static_cast<uint8_t>(move.toRow * 9 + move.toCol)};
}
