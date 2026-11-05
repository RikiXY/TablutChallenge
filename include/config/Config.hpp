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
};

Config parseArguments(int argc, char *argv[]);
