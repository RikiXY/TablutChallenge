#include "engine/TranspositionTable.hpp"
#include "engine/Evaluation.hpp"
#include <atomic>
#include <cstdlib>

namespace {

/**
 * @brief Reads a word of a slot, even while another thread writes it
 *
 * @param word The word to read
 * @return Its value
 */
uint64_t load(const uint64_t &word) {
    return std::atomic_ref<const uint64_t>(word).load(std::memory_order_relaxed);
}

/**
 * @brief Writes a word of a slot, even while another thread reads it
 *
 * @param word The word to write
 * @param value The new value
 */
void save(uint64_t &word, uint64_t value) {
    std::atomic_ref<uint64_t>(word).store(value, std::memory_order_relaxed);
}

/**
 * @brief Packs an entry into the data word of a slot
 *
 * @param entry The entry
 * @return The data word
 */
uint64_t pack(const TranspositionTable::Entry &entry) {
    // Slot::data : generation 8b | bound 8b | depth 8b | score 16b | to 8b | from 8b
    return uint64_t{entry.from} |
           uint64_t{entry.to} << 8 |
           uint64_t{static_cast<uint16_t>(entry.score)} << 16 |
           uint64_t{entry.depth} << 32 |
           uint64_t{static_cast<uint8_t>(entry.bound)} << 40 |
           uint64_t{entry.generation} << 48;
}

/**
 * @brief Unpacks the data word of a slot
 *
 * @param key The key of the position
 * @param data The data word
 * @return The entry
 */
TranspositionTable::Entry unpack(uint64_t key, uint64_t data) {
    return {key,
            static_cast<int16_t>(static_cast<uint16_t>(data >> 16)),
            static_cast<uint8_t>(data >> 32),
            static_cast<Bound>(static_cast<uint8_t>(data >> 40)),
            static_cast<uint8_t>(data >> 48),
            static_cast<uint8_t>(data),
            static_cast<uint8_t>(data >> 8)};
}

} // namespace

Move TranspositionTable::Entry::move() const {
    return makeMove(from, to);
}

TranspositionTable::TranspositionTable(size_t megabytes) {
    resize(megabytes);
}

void TranspositionTable::resize(size_t megabytes) {
    size_t count = 1;
    while (count * 2 * sizeof(Slot) <= megabytes * 1024 * 1024)
        count *= 2;
    slots.assign(count, Slot{});
}

void TranspositionTable::newSearch() {
    generation++;
}

std::optional<TranspositionTable::Entry> TranspositionTable::probe(uint64_t key) const {
    const Slot &slot = slots[key & (slots.size() - 1)]; // key & (slots.size() - 1) = hash % slots.size() but faster because size is a power of 2
    const uint64_t data = load(slot.data);
    if (data == 0 || (load(slot.keyXorData) ^ data) != key)
        return std::nullopt;
    return unpack(key, data);
}

void TranspositionTable::store(uint64_t key, const Move &move, int score, int depth, Bound bound) {
    Slot &slot = slots[key & (slots.size() - 1)]; // key & (slots.size() - 1) = hash % slots.size() but faster because size is a power of 2
    const uint64_t oldData = load(slot.data);
    const Entry old = unpack(load(slot.keyXorData) ^ oldData, oldData);
    if (oldData != 0 && old.key != key && old.generation == generation && old.depth > depth)
        return;

    const Entry entry{key, static_cast<int16_t>(score), static_cast<uint8_t>(depth), bound, generation,
                      move.from, move.to};
    const uint64_t data = pack(entry);
    save(slot.data, data);
    save(slot.keyXorData, key ^ data);
}

int toTable(int score, int ply) {
    if (std::abs(score) <= WIN_BOUND) // Not a win or a loss
        return score;
    return score > 0 ? score + ply : score - ply;
}

int fromTable(int score, int ply) {
    return toTable(score, -ply);
}

bool settles(Bound bound, int score, int alpha, int beta) {
    switch (bound) {
    case Bound::Exact:
        return true;
    case Bound::Lower:
        return score >= beta;
    case Bound::Upper:
        return score <= alpha;
    case Bound::None:
        break;
    }
    return false;
}

Bound boundOf(int score, int alpha, int beta) {
    if (score <= alpha)
        return Bound::Upper;
    if (score >= beta)
        return Bound::Lower;
    return Bound::Exact;
}
