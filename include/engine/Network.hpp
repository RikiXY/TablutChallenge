#pragma once

#include "core/Position.hpp"
#include <string>

// Inputs: king square 0-80, white pawn squares 81-161, black pawn squares 162-242, White to move 243
// Squares are row * 9 + col
constexpr int NETWORK_INPUTS = 244;
constexpr int NETWORK_HIDDEN = 128;

// 125956 bytes
struct Network {
    float inputWeights[NETWORK_INPUTS][NETWORK_HIDDEN];
    float hiddenBiases[NETWORK_HIDDEN];
    float outputWeights[NETWORK_HIDDEN];
    float outputBias;
};

/**
 * @brief Loads the NNUE
 *
 * @param path The weights file
 * @return false if the file can't be read or has the wrong size
 */
bool loadNetwork(const std::string &path);

/**
 * @brief Loads the network from the file in TABLUT_NNUE (if set). Throws if it can't be loaded
 */
void loadNetworkFromEnvironment();

/**
 * @brief Calculates the network's correction value to the linear score
 *
 * @param position The position
 * @return The correction from White's point of view, 0 if no network is loaded
 */
int networkScore(const Position &position);