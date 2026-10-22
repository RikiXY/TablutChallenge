#include "agent/Agent.hpp"

Agent::Agent(std::string role) : Player(role) {}  // richiama il costruttore della classe madre

Action Agent::chooseMove(){
    // TODO selettore mosse AI agent
    return Action("", "", "");
}