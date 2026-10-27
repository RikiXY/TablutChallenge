#pragma once

#include <string>

class Parser{
    private:
    // potrei mettere i parametri come privati ma al momento non c'è bisogno

    public:
    std::string role;
    uint16_t port;
    int timeout;
    std::string ip;
    std::string name;
    std::string playMode;
    Parser(int argc, char *argv[]);
};