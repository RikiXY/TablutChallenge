#include "core/Parser.hpp"
#include <stdexcept>
#include <string_view>

Parser::Parser(int argc, char *argv[]){

    if (argc < 4) 
        throw std::invalid_argument("Missing arguments");

    role = 
        (std::string_view(argv[1]) == "WHITE") 
        ? "WHITE" 
        : (
            (std::string_view(argv[1]) == "BLACK") 
            ? "BLACK" 
            : throw std::invalid_argument("Not a valid role")
        );

    port = 
        (role == "WHITE") 
        ? 5800 
        : 5801; // Ports defined in TablutCompetition/Tablut/src/.../util/Configuration.java

    timeout = std::stoi(argv[2]);

    ip = std::string_view(argv[3]);

    name = 
        (std::string_view(argv[4]) == "-n" || std::string_view(argv[4]) == "--name")
        ? static_cast<std::string>(argv[5])
        : "CustomFish";

    playMode = 
        (std::string_view(argv[6]) == "-m" || std::string_view(argv[6]) == "--mode") 
        ? ((std::string_view(argv[7]) == "manual") 
            ? "manual" 
            : "auto"
        ) 
        : "auto";
}