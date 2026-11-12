#include "engine/Network.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>

namespace {

std::unique_ptr<Network> network; // If null, the network is not loaded and networkScore() returns 0. unique_ptr to automatically free the memory

// The default network, embedded in the binary from the file NETWORK_FILE
asm(".section .rodata\n.balign 64\nembeddedNetwork:\n.incbin \"" NETWORK_FILE "\"\nembeddedNetworkEnd:\n.previous");
extern "C" const char embeddedNetwork[], embeddedNetworkEnd[];

/**
 * @brief Calculates the input of a piece
 *
 * @param piece The piece, not Empty
 * @param square The square of the piece
 * @return The index of the input
 */
int inputOf(Piece piece, int square) {
    switch (piece) {
    case Piece::King:
        return square;
    case Piece::White:
        return 81 + square;
    default:
        return 162 + square;
    }
}

/**
 * @brief Adds a row of input weights to the hidden layer
 *
 * @param hidden The hidden layer
 * @param input The index of the input
 * @param sign 1 to add the row, -1 to subtract it
 */
void addInput(std::array<float, NETWORK_HIDDEN> &hidden, int input, float sign = 1) {
    for (int i = 0; i < NETWORK_HIDDEN; i++)
        hidden[i] += sign * network->inputWeights[input][i];
}

} // namespace

bool loadNetwork(const std::string &path) {
    std::ifstream file(path, std::ios::binary);
    auto loaded = std::make_unique<Network>();
    if (!file.read(reinterpret_cast<char *>(loaded.get()), sizeof(Network)) || file.peek() != EOF)
        return false;
    network = std::move(loaded);
    return true;
}

void loadNetworkFromEnvironment() {
    const char *path = std::getenv("TABLUT_NNUE");
    if (!path) {
        if (embeddedNetworkEnd - embeddedNetwork != sizeof(Network))
            throw std::runtime_error("The embedded network must be exactly " + std::to_string(sizeof(Network)) + " bytes");
        network = std::make_unique<Network>();
        std::memcpy(network.get(), embeddedNetwork, sizeof(Network));
        return;
    }
    if (std::string(path) == "0") {
        network.reset();
        return;
    }
    if (!loadNetwork(path))
        throw std::runtime_error("TABLUT_NNUE: can't load " + std::string(path) + " (it must be exactly " + std::to_string(sizeof(Network)) + " bytes)");
}

bool isNetworkLoaded() {
    return network != nullptr;
}

Accumulator makeAccumulator(const Position &position) {
    Accumulator accumulator;
    accumulator.hidden = network->hiddenBiases;
    for (int square = 0; square < 81; square++) {
        const Piece piece = position.board[square / 9][square % 9];
        if (piece != Piece::Empty)
            addInput(accumulator.hidden, inputOf(piece, square));
    }
    if (position.sideToMove == Side::White)
        addInput(accumulator.hidden, 243);
    return accumulator;
}

Accumulator updateAccumulator(const Accumulator &accumulator, const Position &before, const Position &after) {
    Accumulator updated = accumulator;
    for (int square = 0; square < 81; square++) {
        const Piece old = before.board[square / 9][square % 9];
        const Piece now = after.board[square / 9][square % 9];
        if (old == now)
            continue;
        if (old != Piece::Empty)
            addInput(updated.hidden, inputOf(old, square), -1);
        if (now != Piece::Empty)
            addInput(updated.hidden, inputOf(now, square));
    }
    if (before.sideToMove != after.sideToMove)
        addInput(updated.hidden, 243, after.sideToMove == Side::White ? 1 : -1);
    return updated;
}

int networkScore(const Accumulator &accumulator) {
    if (!network)
        return 0;
    float score = network->outputBias;
    for (int i = 0; i < NETWORK_HIDDEN; i++)
        score += std::clamp(accumulator.hidden[i], 0.0f, 1.0f) * network->outputWeights[i];
    return static_cast<int>(score * 400);
}

int networkScore(const Position &position) {
    if (!network)
        return 0;
    return networkScore(makeAccumulator(position));
}