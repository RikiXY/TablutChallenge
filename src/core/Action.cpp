#include "core/Action.hpp"

#include <bits/stdc++.h>

Action::Action(std::string from, std::string to, std::string turn)
    : from(from), to(to), turn(turn) {}

void Action::toString(){
    jsonFormat = "";   
    jsonFormat += "{\n";
    jsonFormat += "  \"from\": \"" + from + "\",\n";
    jsonFormat += "  \"to\": \"" + to + "\",\n";
    std::transform(turn.begin(), turn.end(), turn.begin(), ::toupper);
    jsonFormat += "  \"turn\": \"" + turn + "\"\n";  // fun fact: funziona anche senza specificare il ruolo, inviando "turn": ""
    jsonFormat += "}";
}

std::string Action::getString(){
    toString();
    return jsonFormat;
}
