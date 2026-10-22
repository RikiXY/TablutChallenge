// === CONVENZIONI DA RISPETTARE ===
// nomi cameCase, no snake_naming
// 1. i casting possono essere eseguiti o tramite reinterpret_cast<TYPE>(OBJ) (o static_cast<TYPE>(OBJ) ) o tramite TYPE(OBJ), niente (TYPE)OBJ

#include <iostream>
#include "connections/ServerConnector.hpp"
#include "core/Game.hpp"

using namespace std;

// #define NAME "CustomFish"

// cmake --build build -j
// ./build/player WHITE 60 127.0.0.1 -n player1 -m manual
// ./build/player BLACK 60 127.0.0.1 -n player2 -m manual

int main(int argc, char *argv[]){

    // TODO: deve passare a 4 e bisogna rimuovere l'opzione di test e usare solo NAME
    if (argc < 8) 
        throw invalid_argument("Missing arguments");

    string role = 
        (string_view(argv[1]) == "WHITE") 
        ? "WHITE" 
        : (
            (string_view(argv[1]) == "BLACK") 
            ? "BLACK" 
            : throw invalid_argument("Not a valid role")
        );

    uint16_t port = 
    (role == "WHITE") 
    ? 5800 
    : 5801; // Ports defined in TablutCompetition/Tablut/src/.../util/Configuration.java

    int timeout = stoi(argv[2]);

    string name = 
    (string_view(argv[4]) == "-n" || string_view(argv[4]) == "--name")
    ? static_cast<string>(argv[5])
    : "CustomFish";

    string playMode = 
    (string_view(argv[6]) == "-m" || string_view(argv[6]) == "--mode") 
    ? ((string_view(argv[7]) == "manual") 
        ? "manual" 
        : "auto"
    ) 
    : "auto";

    ServerConnector connection(argv[3], port);  // argv[3] is IP
    connection.connectToServer();
    cout<<"Connected"<<endl;

    //TODO: far partire il timer qui (?)
    Game game;
    cout<<"Joining game"<<endl;
    game.start(connection, name, role, playMode);  // Join game on <connection> with <name> id
    

    return 0;
}