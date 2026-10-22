#pragma once

#include <string>

class Action{
    private:
    std::string from;
    std::string to;
    std::string turn;

    public:
    Action(std::string, std::string, std::string);
    std::string getString();
};