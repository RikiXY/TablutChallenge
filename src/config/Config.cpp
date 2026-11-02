#include "config/Config.hpp"
#include <stdexcept>
#include <string_view>

Config parseArguments(int argc, char *argv[]) {
    if (argc < 4)
        throw std::invalid_argument("Usage: player <WHITE|BLACK> <timeout_s> <server_ip> [-n name] [-m auto|manual]");

    Config config;

    const std::string_view role(argv[1]);
    if (role == "WHITE")
        config.side = Side::White;
    else if (role == "BLACK")
        config.side = Side::Black;
    else
        throw std::invalid_argument("Not a valid role: " + std::string(role));

    config.port = (config.side == Side::White) ? 5800 : 5801; // Ports defined in TablutCompetition/Tablut/src/.../util/Configuration.java
    config.timeout = std::stoi(argv[2]);
    config.ip = argv[3];

    for (int i = 4; i < argc; i += 2) {
        const std::string_view flag(argv[i]);
        if (i + 1 >= argc)
            throw std::invalid_argument("Missing value for " + std::string(flag));
        const std::string_view value(argv[i + 1]);

        if (flag == "-n" || flag == "--name")
            config.name = value;
        else if (flag == "-m" || flag == "--mode") {
            if (value == "auto")
                config.mode = Mode::Auto;
            else if (value == "manual")
                config.mode = Mode::Manual;
            else
                throw std::invalid_argument("Not a valid mode: " + std::string(value));
        } else
            throw std::invalid_argument("Unknown option: " + std::string(flag));
    }
    return config;
}