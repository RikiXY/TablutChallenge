// === CONVENZIONI DA RISPETTARE ===
// nomi cameCase, no snake_naming
// 1. i casting possono essere eseguiti o tramite reinterpret_cast<TYPE>(OBJ) (o static_cast<TYPE>(OBJ) ) o tramite TYPE(OBJ), niente (TYPE)OBJ

#include <iostream>
#include "connections/ServerConnector.hpp"
#include "core/Game.hpp"

using namespace std;

// #define NAME "CustomFish"

// cmake --build build -j
// ./main WHITE 60 127.0.0.1 -n player1
// ./main BLACK 60 127.0.0.1 -n player2

int main(int argc, char *argv[]){

    // TODO: deve passare a 4 e bisogna rimuovere l'opzione di test e usare solo NAME
    if (argc < 6) 
        throw invalid_argument("Missing arguments");

    string role = string_view(argv[1]) == "WHITE" ? "WHITE" : (string_view(argv[1]) == "BLACK" ? "BLACK" : throw invalid_argument("Not a valid role"));
    uint16_t port = role == "WHITE" ? 5800 : 5801; // Ports defined in TablutCompetition/Tablut/src/.../util/Configuration.java
    int timeout = stoi(argv[2]);
    string name = (string(argv[4]) == "-n" || string(argv[4]) == "--name") ? string(argv[5]) : "CustomFish";

    ServerConnector connection(argv[3], port);  // argv[3] is IP
    connection.connectToServer();
    cout<<"Connected"<<endl;

    //TODO: far partire il timer qui (?)
    Game game;
    cout<<"Joining game"<<endl;
    game.start(connection, name, role);  // Join game on <connection> with <name> id
    

    return 0;
}