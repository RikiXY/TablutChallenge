// === CONVENZIONI DA RISPETTARE ===
// nomi cameCase, no snake_naming
// 1. i casting possono essere eseguiti o tramite reinterpret_cast<TYPE>(OBJ) (o static_cast<TYPE>(OBJ) ) o tramite TYPE(OBJ), niente (TYPE)OBJ

#include <iostream>
#include <string>
#include <sys/socket.h> // necessaria per definire il socket
#include <arpa/inet.h>  // necessaria per instaurare la connessione (i.e. serveAddress , connect(), recv(), ...)
#include <unistd.h>  // necessaria per 

using namespace std;

// #define NAME "CustomFish"

int connect_to_server(string& address, uint16_t& port){
    
    // Establish the connection
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);  // [__domain] AddressFamily_INET = IPv4
                                                         // [__type] SOCK_STREAM = connection-oriented
                                                         // [__protocol] 0 = autodiscovery → TCP
    if (clientSocket == -1) throw domain_error("Creation failed");

    sockaddr_in serverAddress{};  // inserendo {} i campi numerici vengono in automatico inizializzati a 0
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);  // se necessario (diepnde dall'architettura del pc) converte il formato dei byte.
                                           // Host TO Network Short: il network byte order è big-endian (più significativo prima)
                                           // Short perché i valori possibili sono sempre o 5800 o 5801 e bastano 2 byte
    serverAddress.sin_addr.s_addr = inet_addr(address.c_str());  // Assegna al campo s_addr della struttura in_addr (tipologia di sin_addr) l'IPv4 convertito in un intero a 32 bit (network byte order).
                                                                 // l'address di default è 127.0.0.1

    int result = connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));  // C-style cast: (struct sockaddr*)&serverAddress
                                                                                                             //C++-style cast: reinterpret_cast<sockaddr*>(&serverAddress)

    if (result == -1) throw domain_error("Connection failed");
    cout<<"Connected to server"<<endl;

    return clientSocket;
}

void join_game(int& clientSocket, string& name){

    // invio prima la dimensione del nome
    uint32_t nameSize = htonl(uint32_t(name.size()));  // uint32_t perché il protocollo del server prevede che la lunghezza sia rappresentata con esattamente 4 byte e in Network byte order
    send(clientSocket, &nameSize, sizeof(nameSize), 0);  // dichiaro il puntatore, la dimensione del puntatore, e che non ci siano altre flag
    
    // ora invio il nome
    send(clientSocket, name.data(), name.size(), 0);  // mando attraverso il socker i byte del nome e la dimensione del nome completo. non ci sono altri flag

    cout<<"Name sent"<<endl;
}

string receive_json(int& clientSocket){

    uint32_t jsonSize;
    size_t recieved = recv(clientSocket, &jsonSize, sizeof(jsonSize), MSG_WAITALL);
    if (recieved != sizeof(jsonSize))
        throw runtime_error("Failed to recieve all JSON length bytes");
    cout<<"The size of JSON is"<<jsonSize<<" bytes"<<endl;

    jsonSize = ntohl(jsonSize);
    string json(jsonSize, '\0');  // string <name>(<size>, <char>) → create a string filled with <char>
    recieved = recv(clientSocket, &json[0], json.size(), MSG_WAITALL);
    if (recieved != static_cast<size_t>(json.size()))
        throw runtime_error("Failed to receive JSON");
    return json;
}

int main(int argc, char *argv[]){

    // TODO: deve passare a 4 e bisogna rimuovere l'opzione di test e usare solo NAME
    if (argc < 6) throw invalid_argument("Missing arguments");

    uint16_t port = string(argv[1]) == "WHITE" ? 5800 : (string(argv[1]) == "BLACK" ? 5801 : throw invalid_argument("Not a valid role"));  // Ports defined in TablutCompetition/Tablut/src/.../util/Configuration.java
    int timeout = stoi(argv[2]);
    string serverIP = argv[3];
    string name = (string(argv[4]) == "-n" || string(argv[4]) == "--name") ? string(argv[5]) : "CustomFish";


    int clientSocket = connect_to_server(serverIP, port);  // returns file descriptor of socket()
 
    join_game(clientSocket, name);
    cout<<"The server will respond once the other player has connected"<<endl;
    cout<<"Waiting for server response..." << endl;

    string json;
    json = receive_json(clientSocket);
    cout<<"JSON:"<<endl<<json<<endl;

    //TODO: dato il json fare un parser che restituisca lo stato attuale della partita

    cin.get();  // Aspetta INVIO prima di terminare

    close(clientSocket);
    

    return 0;
}