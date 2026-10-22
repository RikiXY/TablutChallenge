#include "agent/Human.hpp"

#include <iostream>
#include "core/Action.hpp"

Human::Human(std::string role): Player(role) {}  // richiama il costruttore della classe madre

Action Human::chooseMove(){
    std::string from;
    std::string to;

    // * validation
    while(1){
        std::cout<<"Enter starting position: "; std::cin>>from;
        std::cout<<"Enter ending position: "; std::cin>>to;
        Action action(from, to, role);

        std::cout<<"Chosen move: "<<std::endl;
        std::cout<<action.getString()<<std::endl;

        if(action.isValid()) 
            return action;
    }
}
