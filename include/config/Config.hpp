#pragma once

#include "core/Position.hpp"
#include <cstdint>
#include <string>

enum class Mode : uint8_t { Auto,
                            Manual };

struct Config { // Default values
    Side side = Side::White;
    uint16_t port = 5800;
    int timeout = 60;
    std::string ip;
    std::string name = "CustomFish";
    Mode mode = Mode::Auto;
    int threads = 1;
};

/**
 * @brief Reads the number of search threads from the TABLUT_THREADS environment variable
 *
 * @param fallback The value to use if TABLUT_THREADS isn't set
 * @return The number of threads. Throws if TABLUT_THREADS isn't a number between 1 and 64
 */
int threadsFromEnvironment(int fallback);

/**
 * @brief Reads the command line options. Throws with the usage if they are wrong
 *
 * @param argc The number of arguments
 * @param argv The arguments
 * @return The options
 */
Config parseArguments(int argc, char *argv[]);
