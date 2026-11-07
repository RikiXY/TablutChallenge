#include "engine/Network.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <stdexcept>

namespace {

std::unique_ptr<Network> network; // If null, the network is not loaded and networkScore() returns 0. unique_ptr to automatically free the memory

/**
 * @brief Adds a row of input weights to the hidden layer
 *
 * @param hidden The hidden layer
 * @param input The index of the input
 */
void addInput(float *hidden, int input) {
    for (int i = 0; i < NETWORK_HIDDEN; i++)
        hidden[i] += network->inputWeights[input][i];
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
    if (path && !loadNetwork(path))
        throw std::runtime_error("TABLUT_NNUE: can't load " + std::string(path) + " (it must be exactly " + std::to_string(sizeof(Network)) + " bytes)");
}

int networkScore(const Position &position) {
    if (!network)
        return 0;

    float hidden[NETWORK_HIDDEN];
    std::copy(std::begin(network->hiddenBiases), std::end(network->hiddenBiases), hidden);
    for (int square = 0; square < 81; square++) {
        switch (position.board[square / 9][square % 9]) {
        case Piece::Empty:
            break;
        case Piece::King:
            addInput(hidden, square);
            break;
        case Piece::White:
            addInput(hidden, 81 + square);
            break;
        case Piece::Black:
            addInput(hidden, 162 + square);
            break;
        }
    }
    if (position.sideToMove == Side::White)
        addInput(hidden, 243);

    float score = network->outputBias;
    for (int i = 0; i < NETWORK_HIDDEN; i++)
        score += std::clamp(hidden[i], 0.0f, 1.0f) * network->outputWeights[i];
    return static_cast<int>(score * 400);
}