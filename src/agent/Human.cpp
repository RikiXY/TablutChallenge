#include "agent/Human.hpp"

#include "core/Action.hpp"
#include <iostream>

Human::Human(ServerConnector &gameConnection, std::string role)
    : Player(gameConnection, role) {} // richiama il costruttore della classe madre

Action Human::chooseMove() {
    std::string from;
    std::string to;

    // ! non è presente validazione delle mosse perché il giocatore umano deve poter sbagliare
    std::cout << "Enter starting position: ";
    std::cin >> from;
    std::cout << "Enter ending position: ";
    std::cin >> to;
    Action action(from, to, role);

    std::cout << "Chosen move: " << std::endl;
    std::cout << action.getString() << std::endl;

    return action;
}
