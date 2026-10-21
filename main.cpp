// === CONVENZIONI DA RISPETTARE ===
// nomi cameCase, no snake_naming
// 1. i casting possono essere eseguiti o tramite reinterpret_cast<TYPE>(OBJ) (o static_cast<TYPE>(OBJ) ) o tramite TYPE(OBJ), niente (TYPE)OBJ

#include <iostream>
#include "include/connections/ServerConnector.hpp"
#include "include/core/Game.hpp"

using namespace std;

// #define NAME "CustomFish"

// g++ -std=c++20 -Iinclude main.cpp $(find src -name "*.cpp") -o main
// ./main WHITE 60 127.0.0.1 -n player1
// ./main BLACK 60 127.0.0.1 -n player2

int main(int argc, char *argv[]){

    // TODO: deve passare a 4 e bisogna rimuovere l'opzione di test e usare solo NAME
    if (argc < 6) 
        throw invalid_argument("Missing arguments");

    uint16_t port = string(argv[1]) == "WHITE" ? 5800 : (string(argv[1]) == "BLACK" ? 5801 : throw invalid_argument("Not a valid role"));  // Ports defined in TablutCompetition/Tablut/src/.../util/Configuration.java
    int timeout = stoi(argv[2]);
    string name = (string(argv[4]) == "-n" || string(argv[4]) == "--name") ? string(argv[5]) : "CustomFish";

    ServerConnection connection(argv[3], port);
    connection.connectToServer();
    cout<<"Connected"<<endl;

    //TODO: far partire il timer qui (?)
    Game game;
    cout<<"Joining game"<<endl;
    game.start(connection, name);  // Join game on <connection> with <name> id
    


    

    
    //TODO: dato il json fare un parser che restituisca lo stato attuale della partita
    

    return 0;
}