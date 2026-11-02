#include "core/Parser.hpp"
#include <stdexcept>
#include <string_view>

Parser::Parser(int argc, char *argv[]){

    if (argc < 4)
        throw std::invalid_argument("Usage: player <WHITE|BLACK> <timeout_s> <server_ip> [-n name] [-m auto|manual]");

    std::string_view roleArg(argv[1]);
    if (roleArg != "WHITE" && roleArg != "BLACK")
        throw std::invalid_argument("Not a valid role: " + std::string(roleArg));
    role = roleArg;

    port = (role == "WHITE") ? 5800 : 5801; // Ports defined in TablutCompetition/Tablut/src/.../util/Configuration.java

    timeout = std::stoi(argv[2]);

    ip = argv[3];

    name = "CustomFish";
    playMode = "auto";

    for (int i = 4; i < argc; i += 2){
        std::string_view flag(argv[i]);
        if (i + 1 >= argc)
            throw std::invalid_argument("Missing value for " + std::string(flag));
        std::string_view value(argv[i + 1]);

        if (flag == "-n" || flag == "--name")
            name = value;
        else if (flag == "-m" || flag == "--mode"){
            if (value != "auto" && value != "manual")
                throw std::invalid_argument("Not a valid mode: " + std::string(value));
            playMode = value;
        }
        else
            throw std::invalid_argument("Unknown option: " + std::string(flag));
    }
}
