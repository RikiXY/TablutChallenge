#pragma once

#include <string>

class Action{
    private:
    std::string from;
    std::string to;
    std::string turn;
    std::string jsonFormat;
    void toString();

    public:
    Action(std::string from, std::string to, std::string turn);
    // ? dovrei mettere il distruttore
    std::string getString();
};