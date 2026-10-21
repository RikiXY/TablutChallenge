#pragma once

#include <string>
#include <array>

class Translator{
    public:
    std::array<std::array<int, 9>, 9> translateFromJson(std::string);  // input json → update current_board

};