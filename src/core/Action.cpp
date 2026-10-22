#include "core/Action.hpp"

#include <bits/stdc++.h>

Action::Action(std::string from, std::string to, std::string turn)
    : from(from), to(to), turn(turn) {}

std::string Action::getString(){
    std::string json = "";
    json += "{\n";
    json += "  \"from\": \"" + from + "\",\n";
    json += "  \"to\": \"" + to + "\",\n";
    std::transform(turn.begin(), turn.end(), turn.begin(), ::toupper);
    json += "  \"turn\": \"" + turn + "\"\n";
    json += "}";
    
    return json;
}